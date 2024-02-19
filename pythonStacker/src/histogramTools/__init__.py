import uproot
import numpy as np
import awkward as ak


# TODO: add convenience functions for rebinning. Really necessary? Prob not

# not sure if class is needed
class Histogramizer:
    '''
    Class to turn TTrees into Numpy Histograms. Should keep track of nominal weights.
    '''
    def __init__(self, inputfile, treename="test"):
        self.inputfile = inputfile
        self.tree: uproot.TTree = uproot.open(self.inputfile + ":" + treename)
        self._weightname = "nominalWeight"
        self._selector = None

    def get_weights(self):
        self._weights = self.tree.arrays([self.weightname], cut=self._selector).array()

    @property
    def weights(self):
        return self._weights

    @weights.setter
    def weights(self, new_weights):
        self._weights = new_weights
        return

    @property
    def weightname(self):
        return self._weightname

    @weightname.setter
    def weightname(self, newname):
        self._weightname = newname
        return

    @property
    def selector(self):
        return self._selector

    @selector.setter
    def selector(self, newselector):
        self._selector = newselector
        return

    def build_histogram(self, variablename, processing=None, aliases=None):
        vars = self.tree.arrays([variablename], cut=self._selector, aliases=aliases)

        print(vars)

    def build_all_histograms(self, list_of_variables):
        for entry in list_of_variables:
            self.build_histogram(entry)
        return
