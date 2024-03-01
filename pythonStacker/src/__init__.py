import numpy as np


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
