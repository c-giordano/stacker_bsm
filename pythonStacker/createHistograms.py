# import numpy as np
# import awkward as ak
import argparse
import sys
import os
import json
import glob

import uproot
import awkward as ak
import src
from src.variables.variableReader import VariableReader, Variable
from src.variables.weightManager import WeightManager
from src.histogramTools import HistogramManager
from submitHistogramCreation import args_add_settingfiles, args_select_specifics, args_add_toggles
from src.configuration import load_uncertainties, Channel, Uncertainty

"""
Script that takes as input a file or set of files, applies cross sections and necessary normalizations (if still needed), and then creates a histogram.
The histogram is taken from settingfiles/Histogramming, so maybe do multiple histograms for the same inputfile? But take range, nbins etc from there as a start.

This is likely code that will be submitted.
Alternatively: create for a single histogram all inputs, so read all files etc.

Must be generic enough: ie. weight variations are easy to support.
More systematic variations is a different question but we can work something out.
"""


def parse_arguments() -> argparse.Namespace:
    """
    For now directly takne from create_histograms in interpretations.
    """
    parser = argparse.ArgumentParser(description='Process command line arguments.')

    # add file arguments
    args_add_settingfiles(parser)
    args_select_specifics(parser)
    args_add_toggles(parser)

    parser.add_argument("--storage", dest="storage", type=str,
                        default="Intermediate", help="Path at which the \
                        histograms are stored")

    # Parse arguments
    args = parser.parse_args()

    if len(sys.argv) <= 1:
        parser.print_help(sys.stderr)
        exit(1)

    if len(args.years) > 1:
        print("Only accept a single year in CreateHistograms. Exiting...")
        exit(1)

    if args.process is None:
        print("Need to specify a process to produce. Exiting...")
        exit(1)

    if args.channel is None:
        print("Need to specify a channel to produce. Exiting...")
        exit(1)

    return args


def prepare_histogram(data, wgts, variable: Variable):
    hist_content, binning, hist_unc = src.histogram_w_unc_flow(ak.to_numpy(data), range=variable.range, wgts=ak.to_numpy(wgts), nbins=variable.nbins)
    return hist_content, binning, hist_unc


def get_histogram_data(variable: Variable, tree, channel: Channel):
    method = variable.get_method()
    data = method(tree, variable.branch_name, channel.selection)
    return data


def create_histogram(variable: Variable, data, weights):
    '''
    TODO
    '''
    # to histogram:
    hist_content, _, hist_unc = prepare_histogram(data, weights, variable)
    return hist_content, hist_unc


def get_tree_from_file(filename, processname) -> uproot.TTree:
    current_rootfile = uproot.open(filename)

    try:
        current_tree: uproot.TTree = current_rootfile[processname]
    except KeyError:
        print(f"{processname} not found in the file {filename}. Trying other keys.")
        for key, classname in current_rootfile.classnames().items():
            if classname != "TTree":
                continue
            if "Namingscheme" in key:
                continue
            current_tree: uproot.TTree = current_rootfile[key]
            break

    return current_tree


def clean_systematics(systematics: dict[str, Uncertainty], process):
    ret = {name: unc for name, unc in systematics.items() if unc.is_process_relevant(process)}
    return ret
            

if __name__ == "__main__":
    # parse arguments
    args = parse_arguments()

    # make sure we have correct storagepath
    storagepath = args.storage

    # initialize variable class:
    variables = VariableReader(args.variablefile, args.variable)

    # prob do same with systematics
    if args.systematic == "weight" or args.systematic == "shape":
        systematics: dict = load_uncertainties(args.systematicsfile, typefilter=args.systematic, allowflat=False)
    elif args.systematic is not None:
        systematics: dict = load_uncertainties(args.systematicsfile, namefilter=args.systematic, allowflat=False)
    else:
        systematics: dict = dict()

    systematics = clean_systematics(systematics, args.process)
    if args.systematic != "shape":
        systematics["nominal"] = Uncertainty("nominal", {})
        systematics["stat_unc"] = Uncertainty("stat_unc", {})

    print(systematics.keys())
    # load process list:
    with open(args.processfile, 'r') as f:
        processfile = json.load(f)
        processinfo = processfile["Processes"][args.process]
        basedir = processfile["Basedir"]

    # prepare channels:
    with open(args.channelfile, 'r') as f:
        # A single job per channel, so we only need to load the current one.
        # This ensures memory usage is refreshed at the end.
        channelinfo = json.load(f)
        channel = Channel(channelinfo[args.channel], channelinfo)
        if channel.isSubchannel:
            print("Current channel is a subchannel. Nothing to be done. Exiting...")
            exit(0)

    # TODO: should this be a global variable?
    globalEFTToggle = args.UseEFT and processinfo.get("isEFT", 0) > 0
    if globalEFTToggle:
        import plugins.eft as eft
        # load names of EFT Variatioons
        # add to weightmanager -> later on
        # already add to systematics:
        eft_variations = eft.getEFTVariationsGroomed()
        for eft_var in eft_variations:
            tmp_dict = {
                "processes": args.process,
                "isEFT": 1
            }
            systematics[eft_var] = Uncertainty(eft_var, tmp_dict)

    # Now also do this for systematic variations? How though?
    # Maybe add an argument for the systematic variations to produce, can define these somewhere.
    # Should be weightvariations here? Maybe also systematic variations or something? idk let's see first for weight variations

    # update storagepath to include channel
    storagepath_main = os.path.join(storagepath, args.channel)
    output_histograms = dict()
    output_histograms[args.channel] = HistogramManager(storagepath_main, args.process, variables, list(systematics.keys()), year=args.years[0])
    for subchannel_name, _ in channel.subchannels.items():
        channel_name = args.channel + subchannel_name
        storagepath_tmp = os.path.join(storagepath, channel_name)
        output_histograms[subchannel_name] = HistogramManager(storagepath_tmp, args.process, variables, list(systematics.keys()), year=args.years[0])

    # TODO: get files based on process names -> processmanager can return this, depending on the sys unc?
    files = []
    for filebase in processinfo["fileglobs"]:
        fileglob = os.path.join(basedir, filebase)
        fileglob += f"*{args.years[0]}"
        if args.systematic == "weight" or args.systematic is None:
            fileglob += "*base.root"
        true_files = glob.glob(fileglob)
        files.extend(true_files)

    for filename in files:
        # filebase should not include a suffix
        # generate basepath with correct folder, folder has timestamp now
        # then:
        # if args.systematic == "shape":
        # first loop systematics, prob just one at a time -> or create filename with "systematic" tag
        # then loop variables
        # if args.systematic == "weight":
        # loop variables
        current_tree: uproot.TTree = get_tree_from_file(filename, args.process)
        # generates masks for subchannels
        subchannelmasks, subchannelnames = channel.produce_masks(current_tree)

        # no structure yet to load systematics weights. Weightmanager? Or move to systematics loop?
        # Does imply more overhead in reading, can try here, see what memory effect it has, otherwise move to systematics.
        # weights = ak.to_numpy(current_tree.arrays(["weights"], cut=channel.selection, aliases={"weights": "nominalWeight"}).weights)
        weights = WeightManager(current_tree, channel.selection, systematics)

        for _, variable in variables.get_variable_objects().items():
            # load data:
            data = get_histogram_data(variable, current_tree, channel)

            print(variable.name)
            for name, syst in systematics.items():
                if name == "stat_unc":
                    # don't need a dedicated run for this; should be filled before
                    continue
                if name == "nominal":
                    hist_content, _, hist_unc = prepare_histogram(data, weights["nominal"], variable)
                    output_histograms[args.channel][variable.name]["nominal"] += hist_content
                    output_histograms[args.channel][variable.name]["stat_unc"] += hist_unc
                else:
                    keys = syst.get_weight_keys()
                    hist_content_up, _, _ = prepare_histogram(data, weights[keys[0]], variable)
                    hist_content_down, _, _ = prepare_histogram(data, weights[keys[1]], variable)
                    output_histograms[args.channel][variable.name][name]["Up"] += hist_content_up
                    output_histograms[args.channel][variable.name][name]["Down"] += hist_content_down
                
                for subchannel_name in subchannelnames:
                    if name == "stat_unc":
                        # don't need a dedicated run for this; should be filled before
                        continue
                    if name == "nominal":
                        hist_content, _, hist_unc = prepare_histogram(data[subchannelmasks[subchannel_name]], weights["nominal"][subchannelmasks[subchannel_name]], variable)
                        output_histograms[subchannel_name][variable.name]["nominal"] += hist_content
                        output_histograms[subchannel_name][variable.name]["stat_unc"] += hist_unc
                    else:
                        keys = syst.get_weight_keys()
                        hist_content_up, _, _ = prepare_histogram(data[subchannelmasks[subchannel_name]], weights[keys[0]][subchannelmasks[subchannel_name]], variable)
                        hist_content_down, _, _ = prepare_histogram(data[subchannelmasks[subchannel_name]], weights[keys[1]][subchannelmasks[subchannel_name]], variable)
                        output_histograms[subchannel_name][variable.name][name]["Up"] += hist_content_up
                        output_histograms[subchannel_name][variable.name][name]["Down"] += hist_content_down

    output_histograms[args.channel].save_histograms()
    for subchannel_name in subchannelnames:
        output_histograms[subchannel_name].save_histograms()
    print("Finished")
    exit(0)
