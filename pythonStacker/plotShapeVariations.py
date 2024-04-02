import argparse
import json
import awkward as ak
import numpy as np
import matplotlib.pyplot as plt
import os

from src.variables.variableReader import VariableReader, Variable
from src.configuration import load_channels
from src.histogramTools import HistogramManager

import src.plotTools.figureCreator as fg
from src import generate_binning
from plotHistograms import modify_yrange_shape, generate_outputfolder, copy_index_html

import plugins.eft as eft

import src.arguments as arguments


def parse_arguments():
    parser = argparse.ArgumentParser(description='Process command line arguments.')

    arguments.add_settingfiles(parser)
    arguments.select_specifics(parser)
    arguments.add_tmp_storage(parser)
    arguments.add_plot_output(parser)

    args = parser.parse_args()
    return args


def lin_quad_plot_EFT(variable: Variable, plotdir: str, histograms, process_info: dict):
    fig, (ax_main, (ax_ratio_one, ax_ratio_two)) = fg.create_multi_ratioplot(n_subplots=2)
    binning = generate_binning(variable.range, variable.nbins)
    # first plot nominal, then start adding variations
    nominal_content = np.array(ak.to_numpy(histograms[variable.name]["nominal"]))
    stat_unc_var = np.nan_to_num(np.array(ak.to_numpy(histograms[variable.name]["stat_unc"])) / nominal_content, nan=0.)

    # pretty_name = generate_process_name("SM", info)
    nominal_weights = np.ones(len(nominal_content))
    ax_main.hist(binning[:-1], binning, weights=nominal_weights, histtype="step", color="k", label="SM")

    eft_variations = eft.getEFTVariationsLinear()
    minim = 1.
    maxim = 1.
    for eft_var in eft_variations:
        wc_factor = 1
        if "ctHRe" in eft_var:
            wc_factor = 20
        if "ctHIm" in eft_var:
            wc_factor = 20
        lin_name = "EFT_" + eft_var
        quad_name = lin_name + "_" + eft_var

        current_variation = nominal_content
        current_variation = current_variation + wc_factor * np.array(ak.to_numpy(histograms[variable.name][lin_name]["Up"]))
        current_variation = current_variation + wc_factor * wc_factor * np.array(ak.to_numpy(histograms[variable.name][quad_name]["Up"]))

        current_variation = np.nan_to_num(current_variation / nominal_content, nan=1.)

        minim = min(minim, np.min(current_variation))
        maxim = max(maxim, np.max(current_variation))
        pretty_eft_name = eft_var + f" = {wc_factor}"
        ax_main.hist(binning[:-1], binning, weights=current_variation, histtype="step", label=pretty_eft_name)

        lin_ratio = np.nan_to_num(wc_factor * np.array(ak.to_numpy(histograms[variable.name][lin_name]["Up"])) / nominal_content, nan=0.)
        quad_ratio = np.nan_to_num(wc_factor * wc_factor * np.array(ak.to_numpy(histograms[variable.name][quad_name]["Up"])) / nominal_content, nan=0.)
        ax_ratio_one.hist(binning[:-1], binning, weights=lin_ratio, histtype="step")
        ax_ratio_two.hist(binning[:-1], binning, weights=quad_ratio, histtype="step")

    ax_main.errorbar(x=binning[:-1] + 0.5 * np.diff(binning), y=np.ones(len(nominal_content)), yerr=stat_unc_var, ecolor='k', label="stat unc.")

    ax_main.set_xlim(variable.range)
    ax_main.set_ylabel("SM + EFT / SM")
    modify_yrange_shape((minim, maxim), ax_main, maxscale=1.4)
    ax_main.legend(ncol=2)

    ax_ratio_one.set_xlim(variable.range)
    ax_ratio_two.set_xlim(variable.range)

    ax_ratio_one.set_ylabel("Lin / SM")
    ax_ratio_two.set_ylabel("Quad / SM")
    ax_ratio_two.set_xlabel(variable.axis_label)

    # fix output name
    fig.savefig(os.path.join(plotdir, f"{variable.name}_ratios.png"))
    fig.savefig(os.path.join(plotdir, f"{variable.name}_ratios.pdf"))
    plt.close(fig)


def main_plot_EFT(variable: Variable, plotdir: str, histograms, process_info: dict):
    fig, ax_main = fg.create_singleplot()

    binning = generate_binning(variable.range, variable.nbins)
    # first plot nominal, then start adding variations
    nominal_content = np.array(ak.to_numpy(histograms[variable.name]["nominal"]))
    stat_unc_var = np.nan_to_num(np.array(ak.to_numpy(histograms[variable.name]["stat_unc"])) / nominal_content, nan=0.)
    # pretty_name = generate_process_name("SM", info)
    nominal_weights = np.ones(len(nominal_content))
    ax_main.hist(binning[:-1], binning, weights=nominal_weights, histtype="step", color="k", label="SM")

    eft_variations = eft.getEFTVariationsLinear()
    minim = 1.
    maxim = 1.
    for eft_var in eft_variations:
        wc_factor = 1
        if "ctHRe" in eft_var:
            wc_factor = 20
        if "ctHIm" in eft_var:
            wc_factor = 20
        lin_name = "EFT_" + eft_var
        quad_name = lin_name + "_" + eft_var

        current_variation = nominal_content
        current_variation = current_variation + wc_factor * np.array(ak.to_numpy(histograms[variable.name][lin_name]["Up"]))
        current_variation = current_variation + wc_factor * wc_factor * np.array(ak.to_numpy(histograms[variable.name][quad_name]["Up"]))

        current_variation = np.nan_to_num(current_variation / nominal_content, nan=1.)

        minim = min(minim, np.min(current_variation))
        maxim = max(maxim, np.max(current_variation))
        pretty_eft_name = eft_var + f" = {wc_factor}"
        ax_main.hist(binning[:-1], binning, weights=current_variation, histtype="step", label=pretty_eft_name)

    ax_main.errorbar(x=binning[:-1] + 0.5 * np.diff(binning), y=np.ones(len(nominal_content)), yerr=stat_unc_var, ecolor='k', label="stat unc.")
    ax_main.set_xlim(variable.range)
    ax_main.set_ylabel("SM + EFT / SM")
    modify_yrange_shape((minim, maxim), ax_main, maxscale=1.4)
    ax_main.legend(ncol=2)
    ax_main.set_xlabel(variable.axis_label)

    # fix output name
    fig.savefig(os.path.join(plotdir, f"{variable.name}.png"))
    fig.savefig(os.path.join(plotdir, f"{variable.name}.pdf"))
    plt.close(fig)


if __name__ == "__main__":
    args = parse_arguments()
    np.seterr(divide='ignore', invalid='ignore')

    # load process specifics
    # need a set of processes
    with open(args.processfile, 'r') as f:
        processinfo = json.load(f)["Processes"][args.process]

    variables = VariableReader(args.variablefile, args.variable)
    channels = load_channels(args.channelfile)
    storagepath = args.storage

    outputfolder_base = generate_outputfolder(args.years, args.outputfolder, suffix="_EFT_Variations")

    # first plot nominal, then start adding variations
    # load variables, want to do this for all processes

    # also load channels

    # contrary to plotHistograms: load uncertainties if no args.UseEFT
    # do need a selection somewhere defined for the uncertainties needed/desired
    for channel in channels:
        if args.channel is not None and channel != args.channel:
            continue

        storagepath_tmp = os.path.join(storagepath, channel)
        systematics = ["nominal", "stat_unc"]

        systematics.extend(eft.getEFTVariationsGroomed())
        outputfolder = os.path.join(outputfolder_base, channel)
        if not os.path.exists(outputfolder):
            os.makedirs(outputfolder)
        copy_index_html(outputfolder)

        histograms = HistogramManager(storagepath_tmp, args.process, variables, systematics, args.years[0])
        histograms.load_histograms()

        for _, variable in variables.get_variable_objects().items():
            lin_quad_plot_EFT(variable, outputfolder, histograms, processinfo)
            main_plot_EFT(variable, outputfolder, histograms, processinfo)

        for subchannel in channels[channel].subchannels.keys():
            storagepath_tmp = os.path.join(storagepath, channel + subchannel)

            outputfolder = os.path.join(outputfolder_base, channel, subchannel)
            if not os.path.exists(outputfolder):
                os.makedirs(outputfolder)
            # if not os.path.exists(outputfolder):
            #     os.makedirs(outputfolder)
            # copy_index_html(outputfolder)

            # for process, info in processinfo.items():
            histograms = HistogramManager(storagepath_tmp, args.process, variables, systematics, args.years[0])
            histograms.load_histograms()
            for _, variable in variables.get_variable_objects().items():
                lin_quad_plot_EFT(variable, outputfolder, histograms, processinfo)
                main_plot_EFT(variable, outputfolder, histograms, processinfo)
