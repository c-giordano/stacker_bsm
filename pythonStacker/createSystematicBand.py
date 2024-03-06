import argparse
import sys
import os
import json
import awkward as ak
import numpy as np

from submitHistogramCreation import args_add_settingfiles, args_select_specifics
from src.configuration import load_uncertainties, load_channels, Uncertainty
from src.variables.variableReader import VariableReader, Variable
from src.histogramTools import HistogramManager

"""
Script to reduce all systematics to a single band. Allows for quicker plotting
"""


def parse_arguments() -> argparse.Namespace:
    """
    For now directly takne from create_histograms in interpretations.
    """
    parser = argparse.ArgumentParser(description='Process command line arguments.')

    # add file arguments
    args_add_settingfiles(parser)
    args_select_specifics(parser)

    parser.add_argument("--storage", dest="storage", type=str,
                        default="Intermediate", help="Path at which the \
                        histograms are stored")

    # Parse arguments
    args = parser.parse_args()

    if len(sys.argv) <= 1:
        parser.print_help(sys.stderr)
        exit(1)

    return args


def get_uncertainty_variation_shape(variable: Variable, uncertainty: Uncertainty, histograms_proc: dict[str, HistogramManager]):
    up = np.zeros(variable.nbins)
    down = np.zeros(variable.nbins)
    if uncertainty.isFlat:
        return up, down

    for process, hists in histograms_proc.items():
        if not uncertainty.is_process_relevant(process):
            continue

        # get the up and down variation:
        up_diff = ak.to_numpy(hists[variable.name][uncertainty.name]["Up"] - hists[variable.name]["nominal"])
        down_diff = ak.to_numpy(hists[variable.name][uncertainty.name]["Down"] - hists[variable.name]["nominal"])

        # keep only variations that are truly showing
        true_up_diff = np.where(up_diff > down_diff, up_diff, down_diff)
        true_up_diff[true_up_diff < 0] = 0.

        true_down_diff = np.where(down_diff < up_diff, down_diff, up_diff)
        true_down_diff[true_down_diff > 0] = 0.

        if uncertainty.correlated_process:
            up += true_up_diff
            down += true_down_diff
        else:
            up += (true_up_diff * true_up_diff)
            down += (true_down_diff * true_down_diff)

    if uncertainty.correlated_process:
        up_sq = np.power(up, 2)
        down_sq = np.power(down, 2)
    else:
        up_sq = up
        down_sq = down

    return up_sq, down_sq


def get_uncertainty_variation_flat(variable: Variable, uncertainty: Uncertainty, histograms_proc: dict[str, HistogramManager]):
    variation = np.zeros(variable.nbins)
    if not uncertainty.isFlat:
        return variation, variation

    for process, hists in histograms_proc.items():
        if not uncertainty.is_process_relevant(process):
            continue

        # get the up and down variation:
        diff = ak.to_numpy(hists[variable.name]["nominal"]) * (1 - uncertainty.rate)
        if uncertainty.correlated_process:
            variation += diff
        else:
            variation += (diff * diff)

    if uncertainty.correlated_process:
        return np.power(variation, 2)
    else:
        return variation, variation

    return


def uncertaintyloop(variable: Variable, histograms_proc: dict[str, HistogramManager], uncertainties: dict[str, Uncertainty], channel: str):
    # NOTE: can automatically do asymm uncertainties in matplotlib!
    uncertainties_squared_up = np.zeros(variable.nbins)
    uncertainties_squared_down = np.zeros(variable.nbins)
    for name, uncertainty in uncertainties.items():
        if not uncertainty.is_channel_relevant(channel):
            print("skip channel")
            # skip irrelevant channels
            continue
        if name == "nominal" or name == "stat_unc":
            continue

        if uncertainty.isFlat:
            up_tmp, down_tmp = get_uncertainty_variation_flat(variable, uncertainty, histograms_proc)
        else:
            # For a single uncertainty, loop processes and add up differences to nominal
            up_tmp, down_tmp = get_uncertainty_variation_shape(variable, uncertainty, histograms_proc)

        uncertainties_squared_up += up_tmp
        uncertainties_squared_down += down_tmp

    # unsquare
    unc_up = np.sqrt(uncertainties_squared_up)
    unc_down = np.sqrt(uncertainties_squared_down)
    return {"Up": unc_up, "Down": unc_down}


def variableloop(variables: VariableReader, histograms_proc: dict[str, HistogramManager], uncertainties: dict, channel: str):
    ret = dict()
    for variable in variables:
        ret[variable.name] = uncertaintyloop(variable, histograms_proc, uncertainties, channel)
    return ret


def channelloop(channels, variables: VariableReader, systematics_shape: dict, systematics_flat: dict):
    uncertainties = {**systematics_shape, **systematics_flat}
    for channelname, info in channels.items():
        # update storagepath to include channel
        storagepath = args.storage
        storagepath = os.path.join(storagepath, channelname)

        histograms_proc: dict[str, HistogramManager] = dict()
        for process, _ in processes.items():
            histograms_proc[process] = HistogramManager(storagepath, process, variables, list(systematics_shape.keys()))
            histograms_proc[process].load_histograms()

        results = variableloop(variables, histograms_proc, uncertainties, channelname)

        # store results for each variable:
        for variable in variables.get_variables():
            tmp_path = os.path.join(storagepath, variable, "total_systematic.parquet")
            ak.to_parquet(ak.Record(results[variable]), tmp_path)


if __name__ == "__main__":
    args = parse_arguments()

    # set up histogrammanager for all processes to be plotted. Load in data. Don't care about variables themselves. Whenever it's defined, it's loaded.

    # go over the systematics, add quadratically unless correlated, then linearly
    # for that part load systematics stuff
    systematics_shape: dict = load_uncertainties(args.systematicsfile, allowflat=False)
    systematics_flat: dict = load_uncertainties(args.systematicsfile, typefilter="flat")
    # should contain all. Define "nominal" and "stat".
    systematics_shape["nominal"] = Uncertainty("nominal", {})
    systematics_shape["stat_unc"] = Uncertainty("stat_unc", {})

    # initialize variable class:
    variables = VariableReader(args.variablefile, args.variable)

    # load process list:
    with open(args.processfile, 'r') as f:
        processfile = json.load(f)
        processes = processfile["Processes"]
        basedir = processfile["Basedir"]

    # prepare channels:
    channels = load_channels(args.channelfile)
    channelloop(channels, variables, systematics_shape, systematics_flat)
