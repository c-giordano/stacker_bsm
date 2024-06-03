# Standard libraries
import numpy as np
np.finfo(np.dtype("float32"))
np.finfo(np.dtype("float64"))

import awkward as ak
import matplotlib.pyplot as plt

import os
import argparse
import json

# Own code
from src import generate_binning, load_prepared_histograms
from src.configuration import load_channels
from src.histogramTools import HistogramManager
from src.variables.variableReader import VariableReader, Variable
import src.arguments as arguments
import src.plotTools.figureCreator as fg

import plugins.eft as eft


def parse_arguments():
    parser = argparse.ArgumentParser(description='Script to plot histograms from the analysis.')

    parser.add_argument("--no_unc", dest="no_unc", action="store_true",
                        default=False, help="Switch to indicate whether the \
                        total uncertainty should be plotted")

    parser.add_argument("--EFT_ratio", dest="EFT_ratio", action="store_true", default=False)
    parser.add_argument("--EFT_fullbkg", dest="EFT_fullbkg", action="store_true", default=False)
    parser.add_argument("--SBRatio", dest="SBRatio", action="store_true", default=False)
    parser.add_argument("--shapes", dest="shapes", action="store_true", default=False)

    arguments.add_settingfiles(parser)
    arguments.select_specifics(parser)
    arguments.add_tmp_storage(parser)
    arguments.add_plot_output(parser)
    arguments.add_toggles(parser)
    args = parser.parse_args()
    return args


def copy_index_html(folder):
    if os.getenv("CMSSW_VERSION"):
        outputfile = os.path.join(folder, "index.php")
        if os.path.exists(outputfile):
            return
        os.system(f"cp /user/nivanden/public_html/index.php {outputfile}")


def generate_outputfolder(years, outputfolder, subdir, suffix=""):
    # outputfolder with year:
    outputsubfolder = ""
    if len(years) == 1:
        outputsubfolder += years[0]
    elif len(years) < 4:
        outputsubfolder += "_".join(years)
    else:
        outputsubfolder += "Run2"

    outputsubfolder += suffix

    outputfolder_base = os.path.join(outputfolder, subdir, outputsubfolder)
    if not os.path.exists(outputfolder_base):
        os.makedirs(outputfolder_base)
    copy_index_html(outputfolder_base)
    copy_index_html(os.path.join(outputfolder, subdir))
    return outputfolder_base


def plot_histograms_base(axis, histograms: dict, variable: Variable, processes: dict, years: list, shapes=False):
    signal = np.zeros(variable.nbins)
    bkg = np.zeros(variable.nbins)
    sum_of_content = np.zeros(variable.nbins)

    binning = generate_binning(variable.range, variable.nbins)

    for name, info in processes.items():
        content = np.zeros(variable.nbins)
        for year in years:
            # fixes ordering based on ordering in json file. Honestly sufficient
            content_tmp = ak.to_numpy(histograms[name][year][variable.name]["nominal"])
            content_tmp = np.array(content_tmp)
            content += content_tmp

        # then add to figure, fix label and color, as well as legend
        pretty_name = ""  # generate_process_name(name, info)
        if shapes:
            axis.hist(binning[:-1], binning, weights=content, histtype="step", color=info["color"], label=pretty_name)
        else:
            axis.bar(x=binning[:-1], height=content, bottom=sum_of_content, width=np.diff(binning), align='edge', linewidth=0, color=info["color"], zorder=-1, label=pretty_name)

        sum_of_content += content
        if int(info.get("isSignal", 0)) > 0:
            signal += content
        else:
            bkg += content

    return {"signal": signal, "bkg": bkg, "sum": sum_of_content, "binning": binning}


def plot_signal_bkg_ratio(axis, binning, signal, background):
    ratio_content = np.nan_to_num(np.divide(signal, background))
    axis.hist(binning[:-1], binning, weights=ratio_content, histtype="step", color="k")
    return {"ratio": ratio_content}


def plot_EFT_line(axis, histograms, variable: Variable, years, operator: str, normalization_contribution=None):
    lin_name = "EFT_" + operator
    quad_name = lin_name + "_" + operator
    binning = generate_binning(variable.range, variable.nbins)
    nominal_content = np.zeros(variable.nbins)
    if normalization_contribution is None:
        normalization_contribution = np.ones(variable.nbins)

    for year in years:
        nominal_content += np.array(ak.to_numpy(histograms[year][variable.name]["nominal"]))

    for wc_factor in [1., 2.]:
        current_variation = nominal_content # nominal_content
        for year in years:
            current_variation = current_variation + wc_factor * np.array(ak.to_numpy(histograms[year][variable.name][lin_name]["Up"]))
            current_variation = current_variation + wc_factor * wc_factor * np.array(ak.to_numpy(histograms[year][variable.name][quad_name]["Up"]))

        pretty_eft_name = operator + f" = {wc_factor}"
        current_variation = np.nan_to_num(current_variation / normalization_contribution, nan=1.)
        axis.hist(binning[:-1], binning, weights=current_variation, histtype="step",
                  label=pretty_eft_name, linewidth=2.)


def finalize_plot(figure, axes, variable: Variable, plotdir: str, plotlabel=""):
    for ax in axes:
        ax.set_xlim(variable.range)

    ax[-1].set_xlabel(variable.axis_label)
    ax[0].text(0.049, 0.77, plotlabel, transform=ax[0].transAxes)

    figure.savefig(os.path.join(plotdir, f"{variable.name}.png"))
    figure.savefig(os.path.join(plotdir, f"{variable.name}.pdf"))
    print(f"Created figure {os.path.join(plotdir, f'{variable.name}.[png/pdf]')}.")
    plt.close(figure)


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


def plotting_sequence(args, histograms, variable, processinfo, plotdir, channel):
    n_ratios = 0
    if args.SBRatio:
        n_ratios = 1
    if args.EFT_ratio:
        n_ratios = 1
    if args.EFT_fullbkg:
        n_ratios = 2

    lumi = get_lumi(args.years)
    fig, axes = fg.create_multi_ratioplot(lumi, True, n_subplots=n_ratios)
    plot_histograms_base(axes[0], histograms, variable, processinfo, args.years, shapes=args.shapes)

    finalize_plot(fig, axes, variable, plotdir, channel)


if __name__ == "__main__":
    args = parse_arguments()
    np.seterr(divide='ignore', invalid='ignore')

    # need a set of processes
    with open(args.processfile, 'r') as f:
        processfile = json.load(f)
        processinfo = processfile["Processes"]
        subbasedir = processfile["Basedir"].split("/")[-1]

    # need a set of variables to plot
    variables = VariableReader(args.variablefile, args.variable)

    # need a set of channels to plot
    # prepare channels:
    channels = load_channels(args.channelfile)

    # set up histogrammanager per process
    storagepath = os.path.join(args.storage, subbasedir)

    # TODO: outputfolder with year and suffix:
    outputfolder_base = generate_outputfolder(args.years, args.outputfolder, subbasedir)

    for channel in channels:
        if args.channel is not None and channel != args.channel:
            continue

        storagepath_tmp = os.path.join(storagepath, channel)
        systematics = ["nominal"]

        outputfolder = os.path.join(outputfolder_base, channel)
        if not os.path.exists(outputfolder):
            os.makedirs(outputfolder)
        copy_index_html(outputfolder)

        histograms = load_prepared_histograms(processinfo, channel, variables, systematics, storagepath_tmp, args)

        for _, variable in variables.get_variable_objects().items():
            if not variable.is_channel_relevant(channel):
                continue
            plot_variable_base(variable, outputfolder, processinfo, histograms, storagepath=storagepath_tmp, years=args.years, drawEFT=args.UseEFT, no_uncertainty=args.no_unc, plotlabel=channel)

        for subchannel in channels[channel].get_subchannels():
            storagepath_tmp = os.path.join(storagepath, channel + subchannel)
            histograms = dict()

            outputfolder = os.path.join(outputfolder_base, channel, subchannel)
            if not os.path.exists(outputfolder):
                os.makedirs(outputfolder)
            copy_index_html(outputfolder)

            histograms = load_prepared_histograms(processinfo, channel, variables, systematics, storagepath_tmp, args)

            for _, variable in variables.get_variable_objects().items():
                if not variable.is_channel_relevant(channel + subchannel):
                    continue
                plot_variable_base(variable, outputfolder, processinfo, histograms, storagepath=storagepath_tmp, years=args.years, drawEFT=args.UseEFT, no_uncertainty=args.no_unc, plotlabel=channel+"_"+subchannel)

    print("Finished!")
