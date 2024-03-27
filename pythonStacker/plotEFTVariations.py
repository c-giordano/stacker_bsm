import awkward as ak
import uproot
import matplotlib.pyplot as plt
import numpy as np

from plugins.eft import buildWeightVariations
from pythonStacker.src.plotTools.figureCreator import create_ratioplot

from src import histogram_w_unc_flow, histogram_w_flow

import argparse
import os

import src.arguments as arguments
from pythonStacker.createEFTWeights import get_eftvariations_filename


def parse_arguments():
    parser = argparse.ArgumentParser(description='Process command line arguments.')

    arguments.add_settingfiles(parser)
    arguments.select_specifics(parser)
    arguments.add_tmp_storage(parser)
    arguments.add_plot_output(parser)
    args = parser.parse_args()
    return args


def loadEFTWeights(inputfile: str, eventclass, storage: str):
    filepath = get_eftvariations_filename(storage, inputfile, eventclass)
    return ak.from_parquet(filepath)


def plotEFTVariations(variableValues, varName, nominalWeights, eftWeights, weightVariations, nbins, range, unit="GeV", scaleVar=None):
    npvar = ak.to_numpy(variableValues)
    nominalWgts = ak.to_numpy(nominalWeights)

    # tmp for under and overflow
    raw_bins = np.linspace(range[0], range[1], nbins + 1)
    use_bins = [np.array([-np.inf]), raw_bins, np.array([np.inf])]
    use_bins = np.concatenate(use_bins)

    base_ratio, binning, unc = histogram_w_unc_flow(npvar, range, nominalWgts, nbins)
    # print("base:", base_ratio)
    # print("unc:", unc)
    # ax.bar(x=binning[:-1], height=upScaleVar - downScaleVar, bottom=downScaleVar, width=np.diff(binning), align='edge', linewidth=0, color='b', alpha=0.25, zorder=-1)

    fig, (ax, ratio) = create_ratioplot()
    ax.hist(binning[:-1], binning, weights=base_ratio, histtype="step", label="SM")
    ax.bar(x=binning[:-1], height=2 * unc, bottom=base_ratio - unc, width=np.diff(binning), align='edge', linewidth=0, color='b', alpha=0.25, zorder=-1)

    # ax.fill_between(binning, base_ratio - unc, base_ratio + unc, step="post", color="k", alpha=0.15)
    ratio.hist(binning[:-1], binning, weights=base_ratio / base_ratio, histtype="step")

    cached_ratios = []
    cache_max = 0.
    for i, (key, val) in enumerate(weightVariations.items()):
        weightVar = nominalWeights
        for ind in val:
            weightVar = weightVar + eftWeights[:, ind]
        weight_as_np = ak.to_numpy(weightVar)

        vals, binning = histogram_w_flow(npvar, range, weight_as_np, nbins)

        if (np.max(vals) > cache_max):
            cache_max = np.max(vals)
        ax.hist(binning[:-1], binning, weights=vals, histtype="step", label=key + "=1")
        cached_ratios.append(vals / base_ratio)
        ratio.hist(binning[:-1], binning, weights=cached_ratios[i], histtype="step")

    ax.legend(ncol=2)

    ax.set_xlim(range)
    ax.set_ylim((0., 1.5 * cache_max))
    ax.set_xticklabels([])

    ratio.set_xlim(range)
    ratio.set_ylim(0.5, np.max(cached_ratios) + 0.5)

    if (unit != "unit" and unit):
        ratio.set_xlabel(r"${}$ [{}]".format(varName, unit))
    else:
        ratio.set_xlabel(varName)
    ax.set_ylabel("Events / {} {}".format(binning[1] - binning[0], unit))
    ratio.set_ylabel("EFT/SM", loc='center')

    fig.savefig("output/plots/" + varName + ".pdf")
    fig.savefig("output/plots/" + varName + ".png")


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument("-f", "--file", dest="inputfile", action="store", required=True,
                        help="Inputfile to use for plotting")
    parser.add_argument("-e", "--eventclass", dest="eventclass", action="store", default=12,
                        help="eventclass to plot.")

    opts, opts_unknown = parser.parse_known_args()

    tree = buildTree(opts.inputfile)

    nominalWeights = tree.arrays(["nominalWeight", "scaleVariations"], f"eventClass=={opts.eventclass}")
    eftWeights = nominalWeights.nominalWeight * loadEFTWeights(opts.inputfile, opts.eventclass)
    weightVariations = buildWeightVariations()

    # print(len(nominalWeights.scaleVariations[:,0]))
    # print(np.max(nominalWeights.scaleVariations[:,0]))
    # plt.scatter(np.linspace(0., 1., len(nominalWeights.scaleVariationsAbs[:,0])), nominalWeights.scaleVariationsAbs[:,1])
    # plt.show()

    # calculate ht:
    jetPts = tree.arrays(["jetPt"], f"eventClass=={opts.eventclass}")
    ht = ak.sum(jetPts.jetPt, axis=1)
    plotEFTVariations(ht, "H_T", nominalWeights.nominalWeight, eftWeights, weightVariations, 23, (250, 1400))

    ptLeps = tree.arrays(["electronPt"], f"eventClass=={opts.eventclass}")
    sortedLeptons = ak.sort(ptLeps.electronPt, axis=1, ascending=False)
    plotEFTVariations(sortedLeptons[:, 0], "p_T-lep1", nominalWeights.nominalWeight, eftWeights, weightVariations, 19, (20, 400))
    plotEFTVariations(sortedLeptons[:, 1], "p_T-lep2", nominalWeights.nominalWeight, eftWeights, weightVariations, 9, (20, 200))

    ftBDT = tree.arrays(["fourTopScore"], f"eventClass=={opts.eventclass}")
    plotEFTVariations(ftBDT.fourTopScore, "BDTscoreTTTT", nominalWeights.nominalWeight, eftWeights, weightVariations, 20, (0, 1))

    stepOne = ftBDT.fourTopScore + 1.
    transf_BDT = np.arctanh(stepOne * 0.5)
    plotEFTVariations(transf_BDT, "transfBDTscoreTTTT", nominalWeights.nominalWeight, eftWeights, weightVariations, 20, (0.5, 3))

    ptmiss = tree.arrays(["ptMiss"], f"eventClass=={opts.eventclass}")
    plotEFTVariations(ptmiss.ptMiss, "PT_miss", nominalWeights.nominalWeight, eftWeights, weightVariations, 20, (0, 500))

    # Need all single entries in "eftVariations", then need the indices of single entries and squared for the cXXX = 1
    # first denote variations, dict: {"ctt":[ind1, ind2]}, those indices are used, so total weight var is nom + eftWeights[:,ind1] + eftWeights[:,ind2]

    # also need part of eft reweighter that gives us naming
    # run parallel for ev class -> define as input parameter? -> argparse!
