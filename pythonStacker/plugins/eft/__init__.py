from plugins.eft.WeightInfo import WeightInfo
from plugins.eft.HyperPoly  import HyperPoly

class eft_reweighter:
    def __init__(self):
        def interpret_weight(weight_id):
            str_s = weight_id.split('_')
            res={}
            for i in range(len(str_s) // 2 ):
                res[str_s[2*i]] = float(str_s[2*i+1].replace('m','-').replace('p','.'))
            return res
        
        self.weightInfo = WeightInfo('plugins/eft/TTTT_MS_reweight_card.pkl')
        self.weightInfo.set_order(2)

        weightInfo_data = list(self.weightInfo.data.items())
        weightInfo_data.sort( key = lambda w: w[1] )
        basepoint_coordinates = map( lambda d: [d[v] for v in self.weightInfo.variables] , map( lambda w: interpret_weight(w[0]), weightInfo_data) )
        lowercase_ids = map( lambda s:s.lower(), self.weightInfo.id )

        ref_point_coordinates = [self.weightInfo.ref_point_coordinates[var] for var in self.weightInfo.variables]
        # print(list(basepoint_coordinates))
        self.hyperPoly  = HyperPoly( self.weightInfo.order )
        self.hyperPoly.initialize( list(basepoint_coordinates), ref_point_coordinates )

        print(self.weightInfo.combinations)
        return
    
    def transform_weights(self, weights):
        coeffs = self.hyperPoly.get_parametrization( weights )
        return coeffs
        
def getEFTVariations():
    weightInfo = WeightInfo('plugins/eft/TTTT_MS_reweight_card.pkl')
    weightInfo.set_order(2)
    return weightInfo.combinations

if __name__ == "__main__":
    print(getEFTVariations())
