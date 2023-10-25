import awkward as ak
import uproot
import matplotlib.pyplot as plt
import mplhep as hep
import numpy as np

from plugins.eft import getEFTVariations

px = 1/plt.rcParams['figure.dpi']

def loadEFTWeights(inputfile, eventclass):
    from prepareEFTWeights import get_eftvariations_filename
    filepath = get_eftvariations_filename(inputfile, eventclass)
    return ak.from_parquet(filepath)

def buildTree(path: str, treesuffix=":test") -> uproot.TTree:
    tree: uproot.TTree = uproot.open(path+treesuffix)
    # does this work as a with 
    return tree

def buildWeightVariations() -> dict:
    eftVariations = getEFTVariations()
    print(eftVariations)
    # get single entries with index to fill ret
    ret = dict()
    for i, entry in enumerate(eftVariations):
        if len(entry) == 1:
            ret[entry[0]] = [i]
        elif len(entry) == 2 and entry[0] == entry[1]:
            ret[entry[0]].append(i)
    
    return ret

def plotEFTVariations(variableValues, varName, nominalWeights, eftWeights, weightVariations, nbins, range, unit="GeV"):
    plt.style.use(hep.style.CMS)
    fig, (ax, ratio) = plt.subplots(2,1, sharex="row", gridspec_kw={'height_ratios': [0.75, 0.25]}, figsize=(800*px, 941*px)) ## 29
    fig.subplots_adjust(left=0.14, right=0.96, top=0.94, bottom=0.12, hspace=0.02)
    fig.align_ylabels([ax, ratio])
    npvar = ak.to_numpy(variableValues)
    nominalWgts = ak.to_numpy(nominalWeights)

    # base_ratio, binning = np.histogram(npvar, bins=nbins, weights=nominalWgts, range=range)

    # tmp for overflow/underflow
    raw_bins = np.linspace(range[0], range[1], nbins+1)
    use_bins = [np.array([-np.inf]), raw_bins, np.array([np.inf])]
    use_bins = np.concatenate(use_bins)

    base_ratio, binning = np.histogram(npvar, bins=use_bins, weights=nominalWgts)
    base_ratio[1]  += base_ratio[0]   ## add underflow to first bin
    base_ratio[-2] += base_ratio[-1]  ## add overflow to last bin
    base_ratio = base_ratio[1:-1]     ## chop off the under/overflow
    binning = raw_bins ## use our original binning (without infinities)

    # end of tmp

    
    ax.hist(binning[:-1], binning, weights=base_ratio, histtype="step", label="SM")
    ratio.hist(binning[:-1], binning, weights=base_ratio/base_ratio, histtype="step")

    cached_ratios = []
    cache_max = 0.
    for i, (key, val) in enumerate(weightVariations.items()):
        weightVar = nominalWeights
        for ind in val:
            weightVar = weightVar + eftWeights[:, ind]
        weight_as_np = ak.to_numpy(weightVar)

        vals, binning = np.histogram(npvar, bins=use_bins, weights=weight_as_np)
        vals[1]  += vals[0]   ## add underflow to first bin
        vals[-2] += vals[-1]  ## add overflow to last bin
        vals = vals[1:-1]     ## chop off the under/overflow
        binning = raw_bins ## use our original binning (without infinities)

        if (np.max(vals) > cache_max): cache_max = np.max(vals)
        ax.hist(binning[:-1], binning, weights=vals, histtype="step", label=key+"=1")
        cached_ratios.append(vals/base_ratio)
        ratio.hist(binning[:-1], binning, weights=cached_ratios[i], histtype="step")

    ax.legend(ncol=2)

    ax.set_xlim(range)
    ax.set_ylim((0., 1.5*cache_max))
    ax.set_xticklabels([])
    ratio.set_xlim(range)
    ratio.set_ylim(0.5, np.max(cached_ratios)+0.5)

    if (unit != "unit" and unit):
        ratio.set_xlabel(r"${}$ [{}]".format(varName, unit))
    else:
        ratio.set_xlabel(varName)
    ax.set_ylabel("Events / {} {}".format(binning[1]-binning[0], unit))
    ratio.set_ylabel("EFT/SM", loc='center')
    # hep.mpl_magic(ax=ax)
    # hep.mpl_magic(ax=ratio)

    hep.cms.label(ax=ax)
    fig.savefig(varName+".pdf") # , bbox_inches='tight') # use a manual option: this is sensitive to variations in y-axes
    fig.savefig(varName+".png") # , bbox_inches='tight') # use a manual option: this is sensitive to variations in y-axes


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument("-f", "--file", dest="inputfile", action="store", required=True,
                        help="Inputfile to use for plotting")
    parser.add_argument("-e", "--eventclass", dest="eventclass", action="store", default=12,
                        help="eventclass to plot.")

    opts, opts_unknown = parser.parse_known_args()


    tree = buildTree(opts.inputfile)
    
    nominalWeights = tree.arrays(["nominalWeight"], f"eventClass=={opts.eventclass}")
    eftWeights = nominalWeights.nominalWeight * loadEFTWeights(opts.inputfile, opts.eventclass) 
    weightVariations = buildWeightVariations()

    # calculate ht:
    jetPts = tree.arrays(["jetPt"], "eventClass==12")
    ht = ak.sum(jetPts.jetPt, axis=1)
    plotEFTVariations(ht, "H_T", nominalWeights.nominalWeight, eftWeights, weightVariations, 24, (200, 1400))

    ptLeps = tree.arrays(["electronPt"], "eventClass==12")
    sortedLeptons = ak.sort(ptLeps.electronPt, axis=1, ascending=False)
    plotEFTVariations(sortedLeptons[:,0], "p_T-lep1", nominalWeights.nominalWeight, eftWeights, weightVariations, 19, (20, 400))
    plotEFTVariations(sortedLeptons[:,1], "p_T-lep2", nominalWeights.nominalWeight, eftWeights, weightVariations, 9, (20, 200))

    ftBDT = tree.arrays(["fourTopScore"], "eventClass==12")
    plotEFTVariations(ftBDT.fourTopScore, "BDTscoreTTTT", nominalWeights.nominalWeight, eftWeights, weightVariations, 20, (0, 1))
    
    stepOne = ftBDT.fourTopScore+1.
    transf_BDT = np.arctanh(stepOne * 0.5)
    plotEFTVariations(transf_BDT, "transfBDTscoreTTTT", nominalWeights.nominalWeight, eftWeights, weightVariations, 20, (0.5, 3))

    ptmiss = tree.arrays(["ptMiss"], "eventClass==12")
    plotEFTVariations(ptmiss.ptMiss, "PT_miss", nominalWeights.nominalWeight, eftWeights, weightVariations, 20, (0, 500))

# 
    # Need all single entries in "eftVariations", then need the indices of single entries and squared for the cXXX = 1
    # first denote variations, dict: {"ctt":[ind1, ind2]}, those indices are used, so total weight var is nom + eftWeights[:,ind1] + eftWeights[:,ind2]

    # also need part of eft reweighter that gives us naming
    # run parallel for ev class -> define as input parameter? -> argparse!