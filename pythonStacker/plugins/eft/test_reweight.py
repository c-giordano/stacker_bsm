from WeightInfo import WeightInfo
from HyperPoly  import HyperPoly
import ROOT

# The class "WeightInfo" instanziated with a pkl file contains information on the base points where the EFT weights are computed. 
# It corresponds to this reweight_card:
# https://github.com/HephyAnalysisSW/genproductions/blob/master/bin/MadGraph5_aMCatNLO/addons/cards/SMEFTsim_topU3l_MwScheme_UFO/TTTT_MS/TTTT_MS_reweight_card.dat
#TTbb: 'TTbb_MS_reweight_card.pkl'
weightInfo = WeightInfo('TTTT_MS_reweight_card.pkl')
weightInfo.set_order(2) # polynomial order

print "Coefficients: %i (%s), order: %i number of weights: %i" %( len(weightInfo.variables), ",".join(weightInfo.variables), weightInfo.order,  weightInfo.nid )
print "Here are the polynomial coefficients we will compute:"+  ", ".join(map( lambda s: "("+",".join( s )+")",  weightInfo.combinations ))
print "This is the reference point the sample was generated at: ", weightInfo.ref_point_coordinates
print "Here are the base points, encoded in the string:"
print weightInfo.id
print

# The following gymnastics is ONLY needed because CMSSW decides, uncomprehensibly, to store all weights in lower case. Not always, but sometimes! 
# This is ambigous (cHB vs. cHb!!), so we can't go by the weight.id for determining the sequence of weights
# Fortunately, WeightInfo knows about the sequence of EFT weights  
def interpret_weight(weight_id):
    str_s = weight_id.split('_')
    res={}
    for i in range(len(str_s)/2):
        res[str_s[2*i]] = float(str_s[2*i+1].replace('m','-').replace('p','.'))
    return res

weightInfo_data = list(weightInfo.data.iteritems())
print weightInfo_data
weightInfo_data.sort( key = lambda w: w[1] )
basepoint_coordinates = map( lambda d: [d[v] for v in weightInfo.variables] , map( lambda w: interpret_weight(w[0]), weightInfo_data) )
lowercase_ids = map( lambda s:s.lower(), weightInfo.id )

# We do not use a reference point, so its coordinates will be zero.
ref_point_coordinates = [weightInfo.ref_point_coordinates[var] for var in weightInfo.variables]

# Translating the weights at the base points (obtained from madgraph) to polynomial coefficients involves solving a matrix equation which is done with 'HyperPoly'.
hyperPoly  = HyperPoly( weightInfo.order )
hyperPoly.initialize( basepoint_coordinates, ref_point_coordinates )

# FWLite from CMSSW
from DataFormats.FWLite import Events, Handle
from PhysicsTools.PythonAnalysis import *
miniAOD = Events(["root://eos.grid.vbc.ac.at//eos/vbc/group/cms/robert.schoefbeck/tttt/miniAOD/TTTT_MS_EFTdecay_schoef-22-05-12-v2-a5d501e738bc46974ac8d371aaff19e9_USER/MINIAODSIMoutput_0.root"]) #change to your location

products = { 'lhe':{'type':'LHEEventProduct', 'label':("externalLHEProducer")}}
handles  = { v:Handle(products[v]['type']) for v in products.keys() }

# first event (do this in a loop)
miniAOD.to(0)
for name, product in products.iteritems():
    miniAOD.getByLabel(products[name]['label'], handles[name])
    products[name]['product'] = handles[name].product()

# This is the FWLite CMSSW GenInfo; we take the weight collection 
lhe_weights = products['lhe']['product'].weights()
weights      = []
for weight in lhe_weights:
    # Store nominal weight (First position!) 
    if weight.id in ['rwgt_1','dummy']: rw_nominal = weight.wgt

    # Some weight.id are lowercase; we decide to keep the weight based on the string but we don't use the string to decide which it is
    if not weight.id.lower() in lowercase_ids: continue 
    weights.append( weight.wgt )

# Now we obtain the coefficients. Store this vector of length weightInfo.nid in the flat ntuple.
coeffs = hyperPoly.get_parametrization( weights )
print "First event below. Compute polynomial weight as a sum of the products of the Wilson coefficients in the 2nd column and the coefficients."
for i_coeff, (comb, coeff) in enumerate(zip( weightInfo.combinations, coeffs)):
    print "%02i %15s Absolute coefficient: %+1.5e relative: %+5.3f"% ( i_coeff, ",".join( comb), coeff, coeff/coeffs[0] ) 

