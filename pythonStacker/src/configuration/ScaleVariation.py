from src.configuration.Uncertainty import Uncertainty
import awkward as ak
import numpy as np


def generate_scalevariations(dict_entry):
    ret = [ScaleVariation("ScaleVar", dict_entry, i) for i in range(6)]
    return ret


def make_envelope(histograms: dict):
    """
    Histograms should be a dict, key is str, other item should be a dict/record containing at least "Up".
    """
    variations = []
    for i in range(6):
        key = f"ScaleVar_{i}"
        variations.append(ak.to_numpy(histograms[key]["Up"]))

    variations_arr = np.array(variations)
    upvar = np.max(variations_arr, axis=0)
    downvar = np.min(variations_arr, axis=0)
    return (upvar, downvar)


class ScaleVariation(Uncertainty):
    def __init__(self, name, dict_entry, instance):
        super().__init__(name, dict_entry)
        self._isFlat = False
        self.type = "weight"
        self._correlated_process = True
        self.correlated_years = True

        self.name = f"ScaleVar_{instance}"
        self.pretty_name = f"ScaleVar_{instance}"
        self.technical_name = f"ScaleVar_{instance}"
        self.weight_key_up = f"ScaleVar_{instance}"
        self.weight_alias_up = f"scaleVariations[:, {instance}]"

        # TODO: implement interpretation of this None
        self.weight_key_down = None
