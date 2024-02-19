import uproot
# import awkward as ak

"""
Helper class to remove consistently loading of weights from file.
It allows for dynamically multiplying weights.
Also support for loading external weights with same dimensions.
"""


def define_systematics():
    ret = {
        "nominal": "nominalWeights",
        "btagging_up": "weightVariations[0]",
        "btagging_down": "weightVariations[1]"
    }
    return ret


def load_weights_to_dict(rootfile: uproot.TTree, systematics: list[str], region_cut: str):
    syst_definitions = dict()
    weights = rootfile.arrays(systematics, cut=region_cut, aliases=syst_definitions)

    return weights
