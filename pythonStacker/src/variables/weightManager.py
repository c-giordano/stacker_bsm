import uproot
import awkward as ak
from src.configuration.Uncertainty import Uncertainty

"""
Helper class to remove consistently loading of weights from file.
It allows for dynamically multiplying weights.
Also support for loading external weights with same dimensions.
"""


class WeightManager():
    def __init__(self, tree: uproot.TTree, selection: str, systematics: dict[str, Uncertainty]):
        self.hasEFT = False
        aliases = self.construct_aliases(systematics)
        keys = list(aliases.keys())
        print(aliases)
        print(keys)
        print(selection)
        self.weights = tree.arrays(keys, cut=selection, aliases=aliases)
        self.add_eftvariations()

    def construct_aliases(self, systematics: dict[str, Uncertainty]):
        aliases = dict()
        aliases["nominal"] = "nominalWeight"
        # each Uncertainty should have a weightVar loaded up
        for name, unc in systematics.items():
            if "EFT_" in name:
                self.hasEFT = True
                continue
            key_unc = unc.get_weight_keys()
            alias_unc = unc.get_weight_aliases()

            tmp = {key_cur: al_cur for key_cur, al_cur in zip(key_unc, alias_unc) if key_cur != "nominal"}
            aliases.update(tmp)
        return aliases

    def add_eftvariations(self):
        if not self.hasEFT:
            return
        filepath = ""
        self.eft_variations = ak.from_parquet(filepath)
        # TODO: extend record self.weights with this new record, or maybe not, idk

    def __getitem__(self, key):
        if "EFT_" in key:
            return self.eft_variations[key]
        return self.weights[key]

    def __setitem__(self, key, value):
        pass
