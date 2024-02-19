import argparse
import sys
import uproot
import awkward as ak
import numpy as np

# from src.configuration import UncertaintyConfig, ProcessConfig

"""
Script to generate a bunch of datacards.
If the inputhistograms don't exist, it will create these.
Need a writer class I guess, needs to 
"""


def arguments():
    parser = argparse.ArgumentParser(description='Process some integers.')

    # rename a bit
    parser.add_argument('-u', '--uncertainty', dest="uncertainty", type=str, help='Path to the uncertainty file')

    # not sure if necessary
    parser.add_argument("-v", '--version', dest="version", type=str, default="", help='Version number of the program')

    # could also be in uncertainty file.
    # maybe change to "datacard setup file"? Containign year, eft flag(?)-> maybe even denote which process
    # then a "which things to include"
    # "region" : {"variable", "ignore_systematics", "nickname?"}
    parser.add_argument("-y", "--year", dest="year", type=int, default=2018, help="Which year the datacard is made for")

    args = parser.parse_args()

    # if len(sys.argv) == 1:
    #     parser.print_help()
    #     parser.exit()

    return args


if __name__ == "__main__":
    # do stuff
    # histograms should already be created, should just need:
    # - which processes
    # - which uncertainties
    #
    # and next make ROOT histograms for them and write to a file
    # and also write lines for these.
    # check combineHarvester!
    # not necessarily easier to use, mainly focused on C++
    #

    args = arguments()

    # output to root:
    outputpath = "output/datacards/test.root"
    hist_content = ak.to_numpy(ak.from_parquet("/home/njovdnbo/Documents/Stacker_v2/pythonStacker/output/Intermediate/HT/SM_TTTT_nominal.parquet"))
    hist_err = ak.to_numpy(ak.from_parquet("/home/njovdnbo/Documents/Stacker_v2/pythonStacker/output/Intermediate/HT/SM_TTTT_uncertainty.parquet"))
    # transform to root:
    import src.histogramTools.converters as cnvrt
    raw_bins = np.linspace(0., 2000., 21)

    ret_th1 = cnvrt.numpy_to_TH1D(hist_content, raw_bins, err=hist_err)
    file = uproot.recreate(outputpath)
    file["test_hist"] = ret_th1
    file["testdir/test_hist"] = ret_th1
    file.close()
    exit()
