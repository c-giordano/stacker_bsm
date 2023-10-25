import uproot
import awkward as ak
import numpy as np
import matplotlib.pyplot as plt
import mplhep as hep

from plugins.eft import eft_reweighter

def  plotBranch():
    return 0

def weightTransformAll(eftVars):
    eft_weight = eft_reweighter()
    var = ak.Array(eft_weight.transform_weights(entry[1:]) for entry in eftVars)

    return var

def loadEFTVarWeights(eventclass) -> ak.Array:
    intputfile = "output/eftWeights/evClass_"+str(eventclass)+"_wgts.parquet"
    return ak.from_parquet(intputfile)

def plotVar(varName, evClass, weightVariations, tree, nbins, range):
    plt.style.use(hep.style.CMS)
    fig, (ax, ratio) = plt.subplots(2,1, sharex="row", gridspec_kw={'height_ratios': [0.75, 0.25]})
    var = tree.arrays([varName], "eventClass=="+str(evClass))
    npvar = ak.to_numpy(var[varName])
    i=0
    base_ratio = 1
    for key, val in weightVariations.items():
        weight_as_np = ak.to_numpy(val)
        vals, binning = np.histogram(npvar, bins=nbins, weights=weight_as_np, range=range)
        if i == 0: base_ratio = vals
        ax.hist(binning[:-1], binning, weights=vals, histtype="step", label=key)
        ratio.hist(binning[:-1], binning, weights=vals/base_ratio, histtype="step")
        i += 1

    ax.legend()
    ratio.set_xlabel(varName)
    plt.savefig(varName+".pdf")
    plt.savefig(varName+".png")


if __name__ == "__main__":
    testfilepath = "testfiles/MainTree_MCPrompt_2023-10-14_16-02_TTTT_EFT_MiniAOD2018_processed_localSubmission_tttt_eft_v1.root"

    tree: uproot.TTree = uproot.open(testfilepath+":test")
    print(type(tree))
    print(tree.keys())
    print(tree["nominalWeight"].array(library="ak"))
    print(len(tree.arrays(["nominalWeight", "jetPt"])))
    print(len(tree.arrays(["nominalWeight", "jetPt"], "eventClass==12", aliases={"ht" : "Sum$(jetPt)"})))
    arrs = tree.arrays(["nominalWeight", "jetPt", "ht"], "eventClass==12", aliases={"ht" : "sum(jetPt, axis=1)"})
    print(arrs)
    arrs = tree.arrays(["nominalWeight", "jetPt", "eftVariationsNorm"], "eventClass==12", aliases={"eftVariationsNorm" : "eftVariations/eftVariations[:,0]"})
    
    #print(eft_weight.transform_weights(arrs.eftVariationsNorm)[0])
    eftVarAtDet = loadEFTVarWeights(12) * arrs.nominalWeight
    # print(arrs.eftVariations/arrs.eftVariations[:,0])
    # print(len(arrs.eftVariations[:,0]))

    # print(eft_weight.transform_weights([3.117e-07, 3.951e-07, 6.816e-07, 1.156e-06, 2.825e-07, 4.593e-07, 3.086e-07, 3.106e-07, 5.184e-07, 8.847e-07, 1.268e-06, 3.774e-07, 5.565e-07, 3.916e-07, 3.936e-07, 1.410e-06, 1.612e-06, 6.871e-07, 8.707e-07, 6.774e-07, 6.793e-07, 3.106e-06, 1.042e-06, 1.419e-06, 1.144e-06, 1.156e-06, 3.156e-07, 4.454e-07, 2.869e-07, 2.819e-07, 7.913e-07, 4.619e-07, 4.601e-07, 3.079e-07, 3.077e-07, 3.097e-07]))

    plt.style.use(hep.style.CMS)
    fig, ax = plt.subplots(1,1)
    ax.hist(np.sum(arrs.jetPt, axis=1), weights=arrs.nominalWeight, histtype="step", bins=20, range=(200,1200), color='r', label="nominal")
    ax.hist(np.sum(arrs.jetPt, axis=1), weights=arrs.nominalWeight+eftVarAtDet[:,4]+eftVarAtDet[:,26], histtype="step", bins=20, range=(200,1200), color='g', label="ctt=1")
    plt.legend()
    plt.savefig("test_ht.png")

    plotVar("fourTopScore", 12, {"Nominal":arrs.nominalWeight, "ctt=1":arrs.nominalWeight+eftVarAtDet[:,4]+eftVarAtDet[:,26]}, tree, 10, (0, 1))
    plotVar("ptMiss", 12, {"Nominal":arrs.nominalWeight, "ctt=1":arrs.nominalWeight+eftVarAtDet[:,4]+eftVarAtDet[:,26]}, tree, 20, (0, 400))

    #print(tree.arrays(["nominalWeight", "jetPt", "ht"], "eventClass==12", aliases={"ht" : "jetPt"}))

