import argparse
import numpy as np
import awkward as ak
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
    parser.add_argument("--no_unc", dest="no_unc", action="store_true",
                        default=False, help="Switch to indicate whether the \
                        total uncertainty should be plotted")

    args_add_settingfiles(parser)
    args_select_specifics(parser)

    args = parser.parse_args()
    return args


def plot_systematics_band(axis, nominal_content, variable: Variable, storagepath: str):
    # get binning:
    binning = generate_binning(variable.range, variable.nbins)

    # load content:
    path_to_unc = os.path.join(storagepath, variable.name, "total_systematic.parquet")
    uncertainty_content = ak.from_parquet(path_to_unc)
    unc_up = np.array(ak.to_numpy(uncertainty_content["Up"]))
    unc_down = np.array(ak.to_numpy(uncertainty_content["Down"]))
    axis.bar(x=binning[:-1], height=unc_up + unc_down, bottom=nominal_content - unc_down, width=np.diff(binning),
             align='edge', linewidth=0, edgecolor='#BEC6C4', alpha=1., zorder=-1, hatch="xxxx", fill=False,
             label="Total unc.")
    return unc_up, unc_down


def generate_process_name(name, processinfo):
    pretty_name = processinfo.get("pretty_name", name)
    if "\\" in pretty_name:
        pretty_name = r"${}$".format(pretty_name)
    elif "t" in pretty_name:
        pretty_name = r"${}$".format(pretty_name)
    return pretty_name


def modify_yrange(total_content, axis, ratio=False):
    cont_max = 1.8 * np.max(total_content)
    cont_min = 0.
    axis.set_ylim((cont_min, cont_max))


def plot_variable_base(variable: Variable, plotdir: str, processes: dict, histograms: dict, storagepath: str, ratio=True, no_uncertainty=False, shapes=False):  # , samplelist: str, plotdir: str, storagepath: str):
    if ratio:
        fig, (ax_main, ax_ratio) = fg.create_ratioplot(lumi=59.8)
    else:
        fig, ax_main = fg.create_singleplot()

    binning = generate_binning(variable.range, variable.nbins)

    if ratio:
        signal = np.zeros(len(binning[:-1]))
        bkg = np.zeros(len(binning[:-1]))

    sum_of_content = np.zeros(variable.nbins)

    for name, info in processes.items():
        # fixes ordering based on ordering in json file. Honestly sufficient
        content = ak.to_numpy(histograms[name][variable.name]["nominal"])
        content = np.array(content)
        # then add to figure, fix label and color, as well as legend
        pretty_name = generate_process_name(name, info)
        if shapes:
            ax_main.hist(binning[:-1], binning, weights=content, histtype="step", color=info["color"], label=pretty_name)
        else:
            ax_main.bar(x=binning[:-1], height=content, bottom=sum_of_content, width=np.diff(binning), align='edge', linewidth=0, color=info["color"], zorder=-1, label=pretty_name)

        sum_of_content += content
        if ratio and int(info.get("isSignal", 0)) > 0:
            signal += content
        elif ratio:
            bkg += content

    if not shapes and not no_uncertainty:
        plot_systematics_band(ax_main, sum_of_content, variable, storagepath)

    if ratio:
        ratio_content = np.nan_to_num(np.divide(signal, bkg))
        ax_ratio.hist(binning[:-1], binning, weights=ratio_content, histtype="step", color="k")

        ax_ratio.set_xlim(variable.range)
        ax_ratio.set_xlim([0, 500])
        ax_ratio.set_xlabel(variable.axis_label)
        ax_ratio.set_ylabel("Signal / bkg")
    else:
        ax_main.set_xlabel(variable.axis_label)

    ax_main.set_xlim(variable.range)
    ax_main.set_ylabel("Events")
    modify_yrange(sum_of_content, ax_main)
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
        if args.channel is not None and channel != args.channel:
            continue
        storagepath_tmp = os.path.join(storagepath, channel)
        systematics = ["nominal"]
        histograms = dict()
        for process, info in processinfo.items():
            histograms[process] = HistogramManager(storagepath_tmp, process, variables, systematics)
            histograms[process].load_histograms()
        for variable in variables:
            plot_variable_base(variable, args.outputfolder, processinfo, histograms, storagepath=storagepath_tmp, no_uncertainty=args.no_unc)
