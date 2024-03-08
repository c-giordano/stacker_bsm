import argparse
import sys
import os
import json
import uproot
import numpy as np

from src.histogramTools import HistogramManager
from src.variables.variableReader import VariableReader, Variable
from src import generate_binning
from src.configuration import load_channels, load_uncertainties, Uncertainty
import src.histogramTools.converters as cnvrt
from src.datacardTools import DatacardWriter

from submitHistogramCreation import args_add_settingfiles  # , args_select_specifics

"""
Script to generate a bunch of datacards.
If the inputhistograms don't exist, it will create these.
Need a writer class I guess, needs to 
"""


def arguments():
    parser = argparse.ArgumentParser(description='Process some integers.')

    args_add_settingfiles(parser)
    # args_select_specifics(parser)

    parser.add_argument("-dcf", "--datacardfile", dest="datacardfile",
                        type=str, help="JSON file with info to create the datacards")
    parser.add_argument("-op", "--outputpath", dest="outputpath",
                        type=str, help="Path to outputfolder for DC.",
                        default="output/datacards/")
    parser.add_argument("--storage", dest="storage", type=str,
                        default="Intermediate", help="Path at which the \
                        histograms are stored")

    # TODO: figure out how to add in EFT stuff.
    # Current idea: use a flag to switch between the types of DCs we write. BSM does require additional config saying which things to combine.

    args = parser.parse_args()

    if len(sys.argv) == 1:
        parser.print_help()
        parser.exit()

    return args


def convert_and_write_histogram(input_histogram, variable: Variable, outputname: str, rootfile: uproot.WritableDirectory, statunc=None):
    raw_bins = generate_binning(variable.range, variable.nbins)
    if statunc is None:
        statunc = np.zeros(len(input_histogram))
    ret_th1 = cnvrt.numpy_to_TH1D(input_histogram, raw_bins, err=statunc)
    rootfile[outputname] = ret_th1


def load_channels_and_subchannels(channelfile):
    channels = load_channels(channelfile)
    ret = dict()
    for channelname, channelinfo in channels.items():
        ret[channelname] = channelinfo
        for subchannelname, info in channelinfo.subchannels.items():
            ret[channelname + subchannelname] = info

    return ret


def get_pretty_channelnames(dc_settings):
    pretty_names = [channel_DC_setting["prettyname"] for _, channel_DC_setting in dc_settings["channelcontent"].items()]
    return pretty_names


if __name__ == "__main__":
    args = arguments()

    # TODO: for systematics, add a year filter or something, so that we don't introduce 10 different config files.
    with open(args.datacardfile, 'r') as f:
        datacard_settings = json.load(f)

    # Load channels
    channels = load_channels_and_subchannels(args.channelfile)

    # Load all processes:
    with open(args.processfile, 'r') as f:
        processes = json.load(f)["Processes"]

    path_to_rootfile = os.path.join(args.outputpath, f"{datacard_settings['DC_name']}.root")
    rootfile = uproot.recreate(path_to_rootfile)

    shape_systematics = load_uncertainties(args.systematicsfile, allowflat=False)
    shape_systematics["nominal"] = Uncertainty("nominal", {})
    shape_systematics["stat_unc"] = Uncertainty("stat_unc", {})

    for channelname, channel_DC_setting in datacard_settings["channelcontent"].items():
        # load histograms for this specific channel and the variable with HistogramManager
        histograms = dict()

        # setup variable reader for the single variable
        var_name = channel_DC_setting["variable"]
        variables = VariableReader(args.variablefile, [var_name])
        storagepath = os.path.join(args.storage, channelname)
        # setup systematics for current channel
        for process in processes.keys():
            if channels[channelname].is_process_excluded(process):
                continue
            histograms = HistogramManager(storagepath, process, variables, list(shape_systematics.keys()), args.years[0])
            histograms.load_histograms()

            # write nominal
            path_to_histogram = f"{channel_DC_setting['prettyname']}/{process}"
            convert_and_write_histogram(histograms[var_name]["nominal"], variables.get_properties(var_name), path_to_histogram, rootfile, statunc=histograms[var_name]["stat_unc"])

            # loop and write systematics
            for systname, syst in shape_systematics.items():
                if systname == "nominal" or systname == "stat_unc":
                    continue
                path_to_histogram_systematic_up = f"{channel_DC_setting['prettyname']}/{syst}Up/{process}"
                path_to_histogram_systematic_down = f"{channel_DC_setting['prettyname']}/{syst}Down/{process}"
                convert_and_write_histogram(histograms[var_name][syst]["Up"], variables.get_properties(var_name), path_to_histogram_systematic_up, rootfile)
                convert_and_write_histogram(histograms[var_name][syst]["Down"], variables.get_properties(var_name), path_to_histogram_systematic_down, rootfile)

    rootfile.close()

    # start txt writing
    path_to_txtfile = os.path.join(args.outputpath, f"{datacard_settings['DC_name']}.txt")
    dc_writer = DatacardWriter(path_to_txtfile)
    dc_writer.initialize_datacard(len(datacard_settings["channelcontent"]), f"{datacard_settings['DC_name']}.root")

    relevant_channels = [channel for name, channel in channels.items() if name in list(datacard_settings["channelcontent"].keys())]
    dc_writer.add_channels(get_pretty_channelnames(datacard_settings), relevant_channels)
    dc_writer.add_processes(list(processes.keys()))

    systematics = load_uncertainties(args.systematicsfile)
    for syst_name, syst_info in systematics.items():
        dc_writer.add_systematic(syst_info)

    dc_writer.write_card()

    exit()
