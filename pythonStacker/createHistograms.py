# import numpy as np
# import awkward as ak
import argparse
import sys
import os
import json

import uproot
import awkward as ak
import src
from src.variables.variableReader import VariableReader, Variable

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

    # generate choices:
    systematic_choices: list = list()
    systematic_choices.append('all')

    parser.add_argument('-v', '--variable', dest='variables', type=str, nargs='+',
                        default=["all"], help='The variables for which histograms are created')
    parser.add_argument('-vf', "--varfile", dest="varfile", type=str,
                        required=True, help="Path to the variable file.")
    parser.add_argument('--sys', '--systematics', dest='systematics', type=str, nargs='+',
                        choices=systematic_choices, default=["all"], help='The systematics variations to create')
    parser.add_argument('-pi', '--processinfo', dest="processinfo", type=str,
                        required=True, help='Path to the processinfo to use')
    parser.add_argument("--process", dest="process", required=True, type=str,
                        help="For which process the histograms need to be created.")
    parser.add_argument("--storage", dest="storage", type=str,
                        default="Intermediate", help="Path at which the \
                        histograms are stored")

    # Parse arguments
    args = parser.parse_args()

    if args.systematics[0] == "all":
        args.systematics = systematic_choices[:-1]

    if len(sys.argv) <= 1:
        parser.print_help(sys.stderr)
        exit(1)

    return args


def prepare_histogram(data, wgts, variable: Variable):
    hist_content, binning, hist_unc = src.histogram_w_unc_flow(data, range=variable.range, wgts=wgts, nbins=variable.nbins)
    return hist_content, binning, hist_unc


def generate_outputname(storagepath: str, variable_nickname: str, process: str, systematic: str = "nominal"):
    """
    Can two arrays be written to a single file?
    """
    outputdir = os.path.join(storagepath, variable_nickname)
    if not os.path.exists(outputdir):
        os.makedirs(outputdir)

    filename = process.replace(" ", "_") + "_" + systematic + ".parquet"
    outputname = os.path.join(outputdir, filename)

    unc_filename = process.replace(" ", "_") + "_uncertainty.parquet"
    uncfile_outputname = os.path.join(outputdir, unc_filename)

    return outputname, uncfile_outputname


def analysis_histograms(processinfo: dict, processname: str, variables: VariableReader, storagepath: str, selection: str):
    basedir = processinfo["Basedir"]
    filelist = processinfo["Processes"][processname]["files"]

    output_content = dict()  # variable: content
    output_unc = dict()
    for file in filelist:
        current_tree = uproot.open(os.path.join(basedir, file))["test"]
        weights = ak.to_numpy(current_tree.arrays("weights", cut=selection, aliases={"weights": "nominalWeight"}).weights)
        # just first do central, worry about systematics later
        print(f"number of variables: {variables.number_of_variables()}")

        for variable in variables.get_variables():
            hist_content, hist_unc = create_histogram(variables.get_properties(variable), current_tree, weights, selection)

            if variable in output_content:
                output_content[variable] += hist_content
                output_unc[variable] += hist_unc
            else:
                output_content[variable] = hist_content
                output_unc[variable] = hist_unc

        # load weights we need based on either systematics or diffeerent. Write function to get right weights
        # load central
        # Data stays the same for each variable in weight variations.
        # So load data once with correct cut, also need to specify the cut somewhere...
        # But then load: the data and the central weights, then for each systematic load the weights, but againthey should be stationary.
        # maybe store the systematic variation weights also somewhere?

    # save output
    for variable in variables.get_variables():
        outputname, unc_outputname = generate_outputname(storagepath, variable, processname)
        ak.to_parquet(output_content[variable], outputname)
        ak.to_parquet(output_unc[variable], unc_outputname)


def create_systematic_histograms(data: ak.Array, systematics: list[str], current_rootfile):
    # return_value = dict()
    # variable: str = ""
    # weights = []
    # for sys in systematics:
    #     # get systematic weights from rootfile
    #     hist_content, hist_unc = create_histogram(variable, current_rootfile, weights)

    return 0


def create_histogram(variable: Variable, tree, weights, selection):
    '''
    TODO
    '''
    # get opts somewhere?
    method = variable.get_method()
    data = method(tree, variable.branch_name, selection)

    # to histogram:
    hist_content, _, hist_unc = prepare_histogram(data, weights, variable)
    return hist_content, hist_unc


if __name__ == "__main__":
    # parse arguments
    args = parse_arguments()

    # make sure we have correct storagepath
    storagepath = args.storage

    # initialize variable class:
    variables = VariableReader(args.varfile, args.variables)

    # prob do same with systematics

    # load process list:
    with open(args.processinfo, 'r') as f:
        processinfo = json.load(f)

    analysis_histograms(processinfo, args.process, variables, storagepath, "eventClass==12")
    # generate histograms for the process
    # Now also do this for systematic variations? How though?
    # Maybe add an argument for the systematic variations to produce, can define these somewhere.
    # Should be weightvariations here? Maybe also systematic variations or something? idk let's see first for weight variations
    # file_list = plot_info["files"]
    # xsecs = plot_info["xsec"]

    # xsecs not relevant for analysis code plotting
    # should also not be multiple files for
    exit(0)
