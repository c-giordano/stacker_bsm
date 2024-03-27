import numpy as np
import uproot
"""
Some general tools to create histograms
"""


def generate_binning(range, nbins):
    raw_bins = np.linspace(range[0], range[1], nbins + 1)
    return raw_bins


def histogram_w_flow(data, range, wgts, nbins):
    """
    Creates a numpy histogram but takes into account the under- and overflow.
    """

    raw_bins = np.linspace(range[0], range[1], nbins + 1)
    use_bins = [np.array([-np.inf]), raw_bins, np.array([np.inf])]
    use_bins = np.concatenate(use_bins)

    binned_data, _ = np.histogram(data, bins=use_bins, weights=wgts)
    binned_data[1] += binned_data[0]   # add underflow to first bin
    binned_data[-2] += binned_data[-1]  # add overflow to last bin
    binned_data = binned_data[1:-1]     # chop off the under/overflow

    return binned_data, raw_bins


def histogram_w_unc_flow(data, range, wgts, nbins):
    """
    Uses:
        - the data to bin
        - range along which to bin
        - Weight for each data point
        - number of bins
    Returns:
        - The binned data
        - The generated binning
        - The absolute uncertainty on each bin

    Creates a numpy histogram. Takes into account over- and underflow. Calculates the MC uncertainties.
    """

    raw_bins = np.linspace(range[0], range[1], nbins + 1)
    use_bins = [np.array([-np.inf]), raw_bins, np.array([np.inf])]
    use_bins = np.concatenate(use_bins)

    binned_data, _ = np.histogram(data, bins=use_bins, weights=wgts)
    sq_binned_data, _ = np.histogram(data, bins=use_bins, weights=wgts * wgts)
    binned_data[1] += binned_data[0]   # add underflow to first bin
    binned_data[-2] += binned_data[-1]  # add overflow to last bin
    binned_data = binned_data[1:-1]     # chop off the under/overflow

    sq_binned_data[1] += sq_binned_data[0]   # add underflow to first bin
    sq_binned_data[-2] += sq_binned_data[-1]  # add overflow to last bin
    sq_binned_data = sq_binned_data[1:-1]     # chop off the under/overflow
    uncertainty = np.sqrt(sq_binned_data)

    return binned_data, raw_bins, uncertainty


"""
ROOTFile management tools:
"""


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
