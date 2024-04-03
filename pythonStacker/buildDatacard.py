import argparse
import sys
import os
import json
import uproot
import numpy as np
import awkward as ak

from src.histogramTools import HistogramManager
from src.variables.variableReader import VariableReader, Variable
from src import generate_binning
from src.configuration import load_channels_and_subchannels, load_uncertainties, Uncertainty
import src.histogramTools.converters as cnvrt
from src.datacardTools import DatacardWriter

import src.arguments as arguments
"""
Script to generate a bunch of datacards.
If the inputhistograms don't exist, it will create these.
Need a writer class I guess, needs to 
"""


def parse_arguments():
    parser = argparse.ArgumentParser(description='Process some integers.')

    arguments.add_settingfiles(parser)
    arguments.add_tmp_storage(parser)
    arguments.add_toggles(parser)
    arguments.add_EFT_choice(parser)
    # args_select_specifics(parser)

    parser.add_argument("-dcf", "--datacardfile", dest="datacardfile",
                        type=str, help="JSON file with info to create the datacards")
    parser.add_argument("-op", "--outputpath", dest="outputpath",
                        type=str, help="Path to outputfolder for DC.",
                        default="output/datacards/")

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

    # safety functions:
    ret_th1 = cnvrt.numpy_to_TH1D(input_histogram, raw_bins, err=statunc)
    rootfile[outputname] = ret_th1


def get_pretty_channelnames(dc_settings):
    pretty_names = [channel_DC_setting["prettyname"] for _, channel_DC_setting in dc_settings["channelcontent"].items()]
    return pretty_names


def nominal_datacard_creation(rootfile: uproot.WritableDirectory, datacard_settings: dict, channels: dict, processes: list, shape_systematics: dict, args: argparse.Namespace):
    """
    Code to create nominal datacard content (ie the really SM processes).
    For SM stuff, this is the only thing that should be used.
    """
    for channelname, channel_DC_setting in datacard_settings["channelcontent"].items():
        # load histograms for this specific channel and the variable with HistogramManager
        histograms = dict()

        # setup variable reader for the single variable
        var_name = channel_DC_setting["variable"]
        variables = VariableReader(args.variablefile, [var_name])
        storagepath = os.path.join(args.storage, channelname)
        # setup systematics for current channel
        for process in processes:
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
                if not syst.is_process_relevant(process):
                    continue
                path_to_histogram_systematic_up = f"{channel_DC_setting['prettyname']}/{syst.technical_name}Up/{process}"
                path_to_histogram_systematic_down = f"{channel_DC_setting['prettyname']}/{syst.technical_name}Down/{process}"
                convert_and_write_histogram(histograms[var_name][systname]["Up"], variables.get_properties(var_name), path_to_histogram_systematic_up, rootfile)
                convert_and_write_histogram(histograms[var_name][systname]["Down"], variables.get_properties(var_name), path_to_histogram_systematic_down, rootfile)


def eft_datacard_creation(rootfile: uproot.WritableDirectory, datacard_settings: dict, eft_variations: list, shape_systematics: dict, args: argparse.Namespace):
    """
    return should be a list of the added "processes" in the datacard.
    """

    ret = [["sm", -2]]
    # do some stuff
    # first add sm to the list and load those histograms -> normal histogrammanager
    # load SM stuff:
    sm_histograms: dict[str, HistogramManager] = dict()
    for channelname, channel_DC_setting in datacard_settings["channelcontent"].items():
        storagepath = os.path.join(args.storage, channelname)

        var_name = channel_DC_setting["variable"]

        variables = VariableReader(args.variablefile, [var_name])

        sm_histograms[channelname] = HistogramManager(storagepath, "TTTT", variables, list(shape_systematics.keys()), args.years[0])
        sm_histograms[channelname].load_histograms()

        path_to_histogram = f"{channel_DC_setting['prettyname']}/sm"
        convert_and_write_histogram(sm_histograms[channelname][var_name]["nominal"], variables.get_properties(var_name), path_to_histogram, rootfile, statunc=sm_histograms[channelname][var_name]["stat_unc"])
        # loop and write systematics
        for systname, syst in shape_systematics.items():
            if systname == "nominal" or systname == "stat_unc":
                continue
            if not syst.is_process_relevant("TTTT"):
                continue
            path_to_histogram_systematic_up = f"{channel_DC_setting['prettyname']}/{systname}Up/sm"
            path_to_histogram_systematic_down = f"{channel_DC_setting['prettyname']}/{systname}Down/sm"
            convert_and_write_histogram(sm_histograms[channelname][var_name][systname]["Up"], variables.get_properties(var_name), path_to_histogram_systematic_up, rootfile)
            convert_and_write_histogram(sm_histograms[channelname][var_name][systname]["Down"], variables.get_properties(var_name), path_to_histogram_systematic_down, rootfile)

    # next lin and quad terms per eft variation
    for eft_var in eft_variations:
        ret.append([f"sm_lin_quad_{eft_var}", -1])
        ret.append([f"quad_{eft_var}", 0])
        lin_name = "EFT_" + eft_var
        quad_name = "EFT_" + eft_var + "_" + eft_var
        for channelname, channel_DC_setting in datacard_settings["channelcontent"].items():
            storagepath = os.path.join(args.storage, channelname)
            # write nominal part

            var_name = channel_DC_setting["variable"]
            variables = VariableReader(args.variablefile, [var_name])

            # load nominal and stat unc? stat unc from EFT sample itself, then nominal is the variation, so:
            content_to_load = ["nominal", "stat_unc", lin_name, quad_name]
            histograms_eft = HistogramManager(storagepath, "TTTT_EFT", variables, content_to_load, args.years[0])
            histograms_eft.load_histograms()

            content_sm_lin_quad_nominal = sm_histograms[channelname][var_name]["nominal"] + histograms_eft[var_name][lin_name]["Up"] + histograms_eft[var_name][quad_name]["Up"]
            statunc_sm_lin_quad_nominal = np.nan_to_num(ak.to_numpy(content_sm_lin_quad_nominal * histograms_eft[var_name]["stat_unc"] / histograms_eft[var_name]["nominal"]))
            path_to_sm_lin_quad = f"{channel_DC_setting['prettyname']}/sm_lin_quad_{eft_var}"
            convert_and_write_histogram(content_sm_lin_quad_nominal, variables.get_properties(var_name), path_to_sm_lin_quad, rootfile, statunc=statunc_sm_lin_quad_nominal)

            content_quad_nominal = histograms_eft[var_name][quad_name]["Up"]
            statunc_quad_nominal = np.nan_to_num(ak.to_numpy(content_quad_nominal * histograms_eft[var_name]["stat_unc"] / histograms_eft[var_name]["nominal"]))
            path_to_quad = f"{channel_DC_setting['prettyname']}/quad_{eft_var}"
            convert_and_write_histogram(content_quad_nominal, variables.get_properties(var_name), path_to_quad, rootfile, statunc=statunc_quad_nominal)

            # loop and write systematics
            for systname, syst in shape_systematics.items():
                if systname == "nominal" or systname == "stat_unc":
                    continue
                if not syst.is_process_relevant("TTTT"):
                    continue

                rel_syst_up = sm_histograms[channelname][var_name][systname]["Up"] / sm_histograms[channelname][var_name]["nominal"]
                rel_syst_down = sm_histograms[channelname][var_name][systname]["Down"] / sm_histograms[channelname][var_name]["nominal"]

                content_sm_lin_quad_syst_up = rel_syst_up * content_sm_lin_quad_nominal
                content_sm_lin_quad_syst_down = rel_syst_down * content_sm_lin_quad_nominal
                path_to_sm_lin_quad_syst_up = f"{channel_DC_setting['prettyname']}/{systname}Down/sm_lin_quad_{eft_var}"
                path_to_sm_lin_quad_syst_down = f"{channel_DC_setting['prettyname']}/{systname}Down/sm_lin_quad_{eft_var}"
                convert_and_write_histogram(content_sm_lin_quad_syst_up, variables.get_properties(var_name), path_to_sm_lin_quad_syst_up, rootfile)
                convert_and_write_histogram(content_sm_lin_quad_syst_down, variables.get_properties(var_name), path_to_sm_lin_quad_syst_down, rootfile)

                content_quad_syst_up = rel_syst_up * content_quad_nominal
                content_quad_syst_down = rel_syst_down * content_quad_nominal
                path_to_quad_syst_up = f"{channel_DC_setting['prettyname']}/{systname}Down/quad_{eft_var}"
                path_to_quad_syst_down = f"{channel_DC_setting['prettyname']}/{systname}Down/quad_{eft_var}"
                convert_and_write_histogram(content_quad_syst_up, variables.get_properties(var_name), path_to_quad_syst_up, rootfile)
                convert_and_write_histogram(content_quad_syst_down, variables.get_properties(var_name), path_to_quad_syst_down, rootfile)

    return ret


if __name__ == "__main__":
    args = parse_arguments()

    # TODO: for systematics, add a year filter or something, so that we don't introduce 10 different config files.
    with open(args.datacardfile, 'r') as f:
        datacard_settings = json.load(f)

    # Load channels
    channels = load_channels_and_subchannels(args.channelfile)

    # Load all processes:
    with open(args.processfile, 'r') as f:
        processes = list(json.load(f)["Processes"].keys())

    path_to_rootfile = os.path.join(args.outputpath, f"{datacard_settings['DC_name']}.root")
    rootfile = uproot.recreate(path_to_rootfile)

    shape_systematics = load_uncertainties(args.systematicsfile, allowflat=False)
    shape_systematics["nominal"] = Uncertainty("nominal", {})
    shape_systematics["stat_unc"] = Uncertainty("stat_unc", {})

    if args.UseEFT:
        eft_part = eft_datacard_creation(rootfile, datacard_settings, ["cQQ1"], shape_systematics, args)
        processes = [process for process in processes if process != "TTTT"]
        processes_write = [[process, i] for i, process in enumerate(processes)]

    nominal_datacard_creation(rootfile, datacard_settings, channels, processes, shape_systematics, args)

    if args.UseEFT:
        processes_write.extend(eft_part)
    # for channelname, channel_DC_setting in datacard_settings["channelcontent"].items():
    #     # load histograms for this specific channel and the variable with HistogramManager
    #     histograms = dict()
    #
    #     # setup variable reader for the single variable
    #     var_name = channel_DC_setting["variable"]
    #     variables = VariableReader(args.variablefile, [var_name])
    #     storagepath = os.path.join(args.storage, channelname)
    #     # setup systematics for current channel
    #     for process in processes.keys():
    #         if channels[channelname].is_process_excluded(process):
    #             continue
    #         histograms = HistogramManager(storagepath, process, variables, list(shape_systematics.keys()), args.years[0])
    #         histograms.load_histograms()
    #
    #         # write nominal
    #         path_to_histogram = f"{channel_DC_setting['prettyname']}/{process}"
    #         convert_and_write_histogram(histograms[var_name]["nominal"], variables.get_properties(var_name), path_to_histogram, rootfile, statunc=histograms[var_name]["stat_unc"])
    #
    #         # loop and write systematics
    #         for systname, syst in shape_systematics.items():
    #             if systname == "nominal" or systname == "stat_unc":
    #                 continue
    #             if not syst.is_process_relevant(process):
    #                 continue
    #             path_to_histogram_systematic_up = f"{channel_DC_setting['prettyname']}/{systname}Up/{process}"
    #             path_to_histogram_systematic_down = f"{channel_DC_setting['prettyname']}/{systname}Down/{process}"
    #             convert_and_write_histogram(histograms[var_name][systname]["Up"], variables.get_properties(var_name), path_to_histogram_systematic_up, rootfile)
    #             convert_and_write_histogram(histograms[var_name][systname]["Down"], variables.get_properties(var_name), path_to_histogram_systematic_down, rootfile)

    rootfile.close()

    # start txt writing
    path_to_txtfile = os.path.join(args.outputpath, f"{datacard_settings['DC_name']}.txt")
    dc_writer = DatacardWriter(path_to_txtfile)
    dc_writer.initialize_datacard(len(datacard_settings["channelcontent"]), f"{datacard_settings['DC_name']}.root")

    relevant_channels = [channel for name, channel in channels.items() if name in list(datacard_settings["channelcontent"].keys())]
    dc_writer.add_channels(get_pretty_channelnames(datacard_settings), relevant_channels)
    dc_writer.add_processes(processes_write)

    systematics = load_uncertainties(args.systematicsfile)
    for syst_name, syst_info in systematics.items():
        dc_writer.add_systematic(syst_info)

    dc_writer.write_card()

    exit()
