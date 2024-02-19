import argparse
import awkward as ak
import matplotlib.pyplot as plt
import json

from create_histograms import generate_outputname
import src.figureCreator as fg
import src.variables as var
from src import generate_binning


def parse_arguments():
    parser = argparse.ArgumentParser(description='Process command line arguments.')

    # generate choices:
    variable_choices: dict = list(var.get_plotting_options("all").keys())
    variable_choices.append('all')
    parser.add_argument('-v', '--variables', dest="variables", type=str,
                        required=True, help='Nickname of variable to plot')
    parser.add_argument('-s', '--samplelist', dest="samplelist", type=str,
                        required=True, help='Path to the samplelist to use')
    parser.add_argument("-o", "--output", dest="outputfolder", action="store", required=False,
                        default="Output/", help="outputfolder to use for plots")
    parser.add_argument("--storage", dest="storage", type=str,
                        default="Intermediate", help="Path at which the \
                        histograms are stored")
    args = parser.parse_args()
    return args


def plot_variable(variable: str, samplelist: str, plotdir: str, storagepath: str):
    fig, ax_main = fg.create_singleplot()

    plot_opts = var.get_plotting_options(variable)
    binning = generate_binning(plot_opts["range"], plot_opts["nbins"])

    for proc_name, processinfo in samplelist.items():
        content, unc = generate_outputname(storagepath, variable, proc_name)
        hist_content = ak.to_numpy(ak.from_parquet(content))
        hist_unc = ak.to_numpy(ak.from_parquet(unc))

        # then add to figure, fix label and color, as well as legend
        ax_main.hist(binning[:-1], binning, weights=hist_content, histtype="step", color=processinfo["color"], label=proc_name)

        # alternatives for uncertainty
        # ax_main.bar(x=binning[:-1], height=2 * hist_unc, bottom=hist_content - hist_unc, width=np.diff(binning), align='edge', linewidth=0, color=processinfo["color"], alpha=0.10, zorder=-1)
        ax_main.errorbar((binning[:-1] + binning[1:]) / 2, hist_content, yerr=hist_unc, fmt='none', ecolor=processinfo["color"])

    ax_main.set_xlim(plot_opts["range"])
    ax_main.set_xlabel(plot_opts["xlabel"])
    ax_main.set_ylabel(plot_opts["ylabel"])

    ax_main.legend()

    # fix output name
    fig.savefig(f"{plotdir}/{variable}.png")
    fig.savefig(f"{plotdir}/{variable}.pdf")
    plt.close(fig)


if __name__ == "__main__":
    args = parse_arguments()
    with open(args.samplelist, 'r') as f:
        plot_info = json.load(f)

    for variable in args.variables:
        plot_variable(variable, plot_info["Processes"], args.outputfolder, args.storage)
