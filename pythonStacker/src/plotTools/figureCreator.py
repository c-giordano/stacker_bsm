import matplotlib.pyplot as plt
import mplhep as hep

px = 1. / plt.rcParams['figure.dpi']


def mod_style():
    params = {
        'ytick.labelsize': 'medium'
    }
    params = {
        'ytick.labelsize': 27,
        'xtick.labelsize': 27,
        'axes.labelsize': 32,
        "axes.linewidth": 1
    }
    # "axes.labelsize": "large",
    plt.rcParams.update(params)


def create_singleplot(lumi=None, wip=True):
    plt.style.use(hep.style.CMS)
    mod_style()
    fig, nominal = plt.subplots(1, 1, figsize=(800 * px, 800 * px))  # 29
    fig.subplots_adjust(left=0.14, right=0.96, top=0.94, bottom=0.12, hspace=0.02)

    label = "WIP" if wip else ""
    if lumi:
        hep.cms.label(ax=nominal, label=label, lumi=lumi, data=True, loc=2)
    else:
        hep.cms.label(ax=nominal, label=label, loc=2)

    return fig, nominal


def create_ratioplot(lumi=None, wip=True):
    """
    Base function to create ratio plots. nominal is large Axes obj used for nominal plotting.
    "ratio" is Axes obj used for plotting the ratio.
    """
    plt.style.use(hep.style.CMS)
    mod_style()
    # fig, (nominal, ratio) = plt.subplots(2, 1, sharex="all", gridspec_kw={'height_ratios': [0.75, 0.25]}, figsize=(800 * px, 941 * px))  # 29
    fig, (nominal, ratio) = plt.subplots(2, 1, sharex="all", gridspec_kw={'height_ratios': [1., 0.25]}, figsize=(800 * px, 918 * px))  # 29
    fig.subplots_adjust(left=0.14, right=0.96, top=0.94, bottom=0.12, hspace=0.045)
    fig.align_ylabels([nominal, ratio])

    if lumi:
        label = "Work in progress" if wip else ""
        hep.cms.label(ax=nominal, label=label, lumi=lumi, data=True, loc=2)
    else:
        label = "WIP" if wip else ""
        hep.cms.label(ax=nominal, label=label, loc=2)

    return fig, (nominal, ratio)


def create_multi_ratioplot(lumi=None, wip=True, n_subplots=1):
    """
    Base function to create multiple ratio plots. "nominal" is large Axes obj used for nominal plotting.
    "ratios" is a list of Axes obj used for plotting the ratios.
    """
    plt.style.use(hep.style.CMS)
    mod_style()
    fig, axs = plt.subplots(n_subplots + 1, 1, sharex="row", gridspec_kw={'height_ratios': [0.75] + [0.25] * n_subplots}, figsize=(800 * px, 941 * px + 191 * px * (n_subplots - 1)))  # 29
    fig.subplots_adjust(left=0.14, right=0.96, top=1. - (0.06 / (0.75 + (n_subplots * 0.25))), bottom=0.12 / (0.75 + (n_subplots * 0.25)), hspace=0.02 * (0.75 + (n_subplots * 0.25)))
    fig.align_ylabels(axs)

    nominal = axs[0]
    ratios = axs[1:]

    label = "WIP" if wip else ""
    if lumi:
        hep.cms.label(ax=nominal, label=label, lumi=lumi, data=True, loc=1)
    else:
        hep.cms.label(ax=nominal, label=label, loc=1)

    return fig, (nominal, ratios)


if __name__ == "__main__":
    # testing code
    import matplotlib.pyplot as plt
    import numpy as np

    def test_create_multi_ratioplot():
        # Create some random data
        x = np.linspace(0, 10, 100)
        y = np.random.rand(100)

        # Create the plot
        fig, (nominal, ratios) = create_multi_ratioplot(lumi=3000, wip=True, n_subplots=2)

        # Plot the random data
        nominal.plot(x, y)

        # Save the plot as a PNG file
        fig.savefig("test_plot.png")

    def test_create_ratioplot():
        # Create some random data
        x = np.linspace(0, 10, 100)
        y = np.random.rand(100)

        # Create the plot
        fig, (nominal, ratio) = create_ratioplot(lumi=3000, wip=True)

        # Plot the random data
        nominal.plot(x, y)

        # Save the plot as a PNG file
        fig.savefig("test_ratio_plot.png")

    # Run the test
    test_create_ratioplot()

    # Run the test
    test_create_multi_ratioplot()
    exit()
