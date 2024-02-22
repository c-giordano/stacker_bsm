import ROOT
import numpy as np

# methods to take a numpy histogram -> ROOT histogram and vice versa. Other methods could be added.


def numpy_to_TH1D(hist, bins, err=None) -> ROOT.TH1D:
    # Create a TH1D object
    th1d = ROOT.TH1D("hist", "hist", len(bins) - 1, bins)

    # Fill the TH1D object with the histogram data
    for i in range(len(hist)):
        th1d.SetBinContent(i + 1, hist[i])
        if not err is None:
            th1d.SetBinError(i + 1, err[i])
    # th1d.Draw()

    return th1d


def TH1D_to_numpy(th1d: ROOT.TH1D):
    # Get the number of bins
    nbins = th1d.GetNbinsX()

    # Create empty arrays for the histogram data and the bin edges
    hist = np.zeros(nbins)
    bins = np.zeros(nbins + 1)
    err = np.zeros(nbins)

    # Fill the arrays with the data from the TH1D object
    for i in range(nbins):
        hist[i] = th1d.GetBinContent(i + 1)
        bins[i] = th1d.GetBinLowEdge(i + 1)
        err[i] = th1d.GetBinError(i + 1)
    bins[nbins] = th1d.GetBinLowEdge(nbins) + th1d.GetBinWidth(nbins)

    return hist, bins, err


if __name__ == "__main__":
    def test_numpy_to_TH1D():
        # Create a numpy histogram
        hist, bins = np.histogram(np.random.normal(size=1000))

        # Convert the numpy histogram to a ROOT.TH1D object
        th1d = numpy_to_TH1D(hist, bins)

        # Check that the TH1D object has the correct number of bins
        assert th1d.GetNbinsX() == len(bins) - 1

        # Check that the TH1D object has the correct bin content
        for i in range(len(hist)):
            assert th1d.GetBinContent(i + 1) == hist[i]

    # Run the test
    test_numpy_to_TH1D()

    def test_TH1D_to_numpy():
        # Create a ROOT.TH1D histogram
        hist = ROOT.TH1D("hist", "hist", 100, -5, 5)
        for i in range(1000):
            hist.Fill(np.random.normal())

        # Convert the ROOT.TH1D histogram to a numpy histogram
        numpy_hist, bins, err = TH1D_to_numpy(hist)

        # Check that the numpy histogram has the correct number of bins
        assert len(numpy_hist) == hist.GetNbinsX()

        # Check that the numpy histogram has the correct bin content
        for i in range(hist.GetNbinsX()):
            assert numpy_hist[i] == hist.GetBinContent(i + 1)

        # Check that the numpy histogram has the correct bin edges
        for i in range(hist.GetNbinsX() + 1):
            assert bins[i] == hist.GetBinLowEdge(i + 1) if i != hist.GetNbinsX() else bins[i] == hist.GetBinLowEdge(i) + hist.GetBinWidth(i)

        # Check that the numpy histogram has the correct bin errors
        for i in range(hist.GetNbinsX()):
            assert err[i] == hist.GetBinError(i + 1)

    # Run the test
    test_TH1D_to_numpy()
