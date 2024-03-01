import argparse
import numpy as np
import matplotlib.pyplot as plt
import json
import os

from src.variables.variableReader import VariableReader, Variable
from src.configuration import load_channels
from src.histogramTools import HistogramManager
import src.plotTools.figureCreator as fg
from src import generate_binning
from submitHistogramCreation import args_add_settingfiles, args_select_specifics


def parse_arguments():
    parser = argparse.ArgumentParser(description='Process command line arguments.')

    # generate choices:
    # kinda think need to change this to same arguments as submitHistogram stuff
    # variable_choices: dict = list(var.get_plotting_options("all").keys())
    # variable_choices.append('all')
    # parser.add_argument('-v', '--variables', dest="variables", type=str,
    #                     required=True, help='Nickname of variable to plot')
    # parser.add_argument('-s', '--samplelist', dest="samplelist", type=str,
    #                     required=True, help='Path to the samplelist to use')
    parser.add_argument("-o", "--output", dest="outputfolder", action="store", required=False,
                        default="output/", help="outputfolder to use for plots")
    parser.add_argument("--storage", dest="storage", type=str,
                        default="Intermediate", help="Path at which the \
                        histograms are stored")

    args_add_settingfiles(parser)
    args_select_specifics(parser)

    args = parser.parse_args()
    return args


def plot_variable_base(variable: Variable, plotdir: str, processes: dict, histograms: dict, ratio=True):  # , samplelist: str, plotdir: str, storagepath: str):
    if ratio:
        fig, (ax_main, ax_ratio) = fg.create_ratioplot()
    else:
        fig, ax_main = fg.create_singleplot()

    binning = generate_binning(variable.range, variable.nbins)

    if ratio:
        signal = np.zeros(len(binning[:-1]))
        bkg = np.zeros(len(binning[:-1]))

    for name, info in processes.items():
        # fixes ordering based on ordering in json file. Honestly sufficient

        content = histograms[name][variable.name]["nominal"]

        # then add to figure, fix label and color, as well as legend
        ax_main.hist(binning[:-1], binning, weights=content, histtype="step", color=info["color"], label=name)

        if ratio and info.get("isSignal", 0) > 0:
            signal += content
        elif ratio:
            bkg += content
        
        # todo: uncertainty
        # alternatives for uncertainty
        # ax_main.bar(x=binning[:-1], height=2 * hist_unc, bottom=hist_content - hist_unc, width=np.diff(binning), align='edge', linewidth=0, color=processinfo["color"], alpha=0.10, zorder=-1)
        # ax_main.errorbar((binning[:-1] + binning[1:]) / 2, hist_content, yerr=hist_unc, fmt='none', ecolor=processinfo["color"])

    if ratio:
        ratio_content = np.divide(signal, bkg)
        ax_ratio.hist(binning[:-1], binning, weights=ratio_content, histtype="step", color="k")

        ax_ratio.set_xlim(variable.range)
        ax_ratio.set_xlabel(variable.axis_label)
        ax_ratio.set_ylabel("Signal / bkg")

    else:
        ax_main.set_xlabel(variable.axis_label)

    ax_main.set_xlim(variable.range)
    ax_main.set_ylabel("Events")

    ax_main.legend()

    # fix output name
    fig.savefig(os.path.join(plotdir, f"{variable.name}.png"))
    fig.savefig(os.path.join(plotdir, f"{variable.name}.pdf"))
    plt.close(fig)


if __name__ == "__main__":
    args = parse_arguments()

    # need a set of processes
    with open(args.processfile, 'r') as f:
        processinfo = json.load(f)["Processes"]

    # need a set of variables to plot
    variables = VariableReader(args.variablefile, args.variable)

    # need a set of channels to plot
    # prepare channels:
    channels = load_channels(args.channelfile)

    # set up histogrammanager per process
    storagepath = args.storage

    # per channel
    # per variable
    # run plot loop
    for channel in channels:
        storagepath_tmp = os.path.join(storagepath, channel)
        systematics = ["nominal"]
        histograms = dict()
        for process, info in processinfo.items():
            histograms[process] = HistogramManager(storagepath_tmp, process, variables, systematics)
            histograms[process].load_histograms()
        for variable in variables:
            plot_variable_base(variable, args.outputfolder, processinfo, histograms)
