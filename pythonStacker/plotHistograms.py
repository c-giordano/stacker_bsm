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
import src.arguments as arguments


def parse_arguments():
    parser = argparse.ArgumentParser(description='Process command line arguments.')

    parser.add_argument("--no_unc", dest="no_unc", action="store_true",
                        default=False, help="Switch to indicate whether the \
                        total uncertainty should be plotted")

    arguments.add_settingfiles(parser)
    arguments.select_specifics(parser)
    arguments.add_tmp_storage(parser)
    arguments.add_plot_output(parser)
    args = parser.parse_args()
    return args


def copy_index_html(folder):
    if os.getenv("CMSSW_VERSION"):
        outputfile = os.path.join(folder, "index.php")
        if os.path.exists(outputfile):
            return
        os.system(f"cp /user/nivanden/public_html/index.php {outputfile}")


def plot_systematics_band(axis, nominal_content, variable: Variable, storagepath: str, years: list):
    # get binning:
    binning = generate_binning(variable.range, variable.nbins)

    # load content:
    outputfilename = "total_systematic_"
    if len(years) == 1:
        outputfilename += years[0]
    elif len(years) < 4:
        outputfilename += "_".join(years)
    else:
        outputfilename += "all"
    outputfilename += ".parquet"

    path_to_unc = os.path.join(storagepath, variable.name, outputfilename)
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


def modify_yrange_shape(total_content, axis, maxscale=1.8):
    cont_max = maxscale * np.max(total_content)
    cont_min = 0.8 * np.min(total_content)
    axis.set_ylim((cont_min, cont_max))


def get_lumi(years):
    total_lumi = 0.
    lumi = {
        "2016PreVFP": 19.5,
        "2016PostVFP": 16.8,
        "2016": 36.3,
        "2017": 41.5,
        "2018": 59.8,
    }
    for year in years:
        total_lumi += lumi[year]

    if total_lumi >= 100.:
        total_lumi = round(total_lumi)
    return total_lumi


def generate_outputfolder(years, outputfolder, suffix=""):
    # outputfolder with year:
    outputsubfolder = ""
    if len(years) == 1:
        outputsubfolder += "_" + years[0]
    elif len(years) < 4:
        outputsubfolder += "_" + "_".join(years)
    else:
        outputsubfolder += "_Run2"

    outputsubfolder += suffix

    outputfolder_base = os.path.join(outputfolder, outputsubfolder)
    if not os.path.exists(outputfolder_base):
        os.makedirs(outputfolder_base)
    copy_index_html(outputfolder_base)
    return outputfolder_base


def plot_variable_base(variable: Variable, plotdir: str, processes: dict, histograms: dict, storagepath: str, years: list, ratio=True, no_uncertainty=False, shapes=False):  # , samplelist: str, plotdir: str, storagepath: str):
    if ratio:
        fig, (ax_main, ax_ratio) = fg.create_ratioplot(lumi=get_lumi(years))
    else:
        fig, ax_main = fg.create_singleplot(lumi=get_lumi(years))

    binning = generate_binning(variable.range, variable.nbins)

    if ratio:
        signal = np.zeros(len(binning[:-1]))
        bkg = np.zeros(len(binning[:-1]))

    sum_of_content = np.zeros(variable.nbins)

    for name, info in processes.items():
        content = np.zeros(variable.nbins)
        for year in years:
            # fixes ordering based on ordering in json file. Honestly sufficient
            content_tmp = ak.to_numpy(histograms[name][year][variable.name]["nominal"])
            content_tmp = np.array(content_tmp)
            content += content_tmp

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
        plot_systematics_band(ax_main, sum_of_content, variable, storagepath, years)

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
    ax_main.legend(ncols=2)

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

    # outputfolder with year:
    outputfolder_base = generate_outputfolder(args.years, args.outputfolder)

    print(channels)
    for channel in channels:
        if args.channel is not None and channel != args.channel:
            continue

        storagepath_tmp = os.path.join(storagepath, channel)
        systematics = ["nominal"]
        histograms = dict()

        outputfolder = os.path.join(outputfolder_base, channel)
        if not os.path.exists(outputfolder):
            os.makedirs(outputfolder)
        copy_index_html(outputfolder)

        for process, info in processinfo.items():
            histograms[process] = dict()
            for year in args.years:
                histograms[process][year] = HistogramManager(storagepath_tmp, process, variables, systematics, year)
                histograms[process][year].load_histograms()
        for _, variable in variables.get_variable_objects().items():
            plot_variable_base(variable, outputfolder, processinfo, histograms, storagepath=storagepath_tmp, years=args.years, no_uncertainty=args.no_unc)

        for subchannel in channels[channel].subchannels.keys():
            storagepath_tmp = os.path.join(storagepath, channel + subchannel)
            histograms = dict()

            outputfolder = os.path.join(outputfolder_base, channel, subchannel)
            if not os.path.exists(outputfolder):
                os.makedirs(outputfolder)
            copy_index_html(outputfolder)

            for process, info in processinfo.items():
                histograms[process] = dict()
                for year in args.years:
                    histograms[process][year] = HistogramManager(storagepath_tmp, process, variables, systematics, year)
                    histograms[process][year].load_histograms()
            for _, variable in variables.get_variable_objects().items():
                plot_variable_base(variable, outputfolder, processinfo, histograms, storagepath=storagepath_tmp, years=args.years, no_uncertainty=args.no_unc)
