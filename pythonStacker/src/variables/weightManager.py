import uproot
import awkward as ak

"""
Helper class to remove consistently loading of weights from file.
It allows for dynamically multiplying weights.
Also support for loading external weights with same dimensions.
"""


class WeightManager():
    def __init__(self, file: uproot.ReadOnlyDirectory, xsec: float, weightExpr: str = "genWeight", normExpr: str = "genEventSumw", lumi: float = 138000., cut: str = None):
        self.xsec: float = xsec

        weightNorm: ak.Array = file["Runs"].arrays(["norm"], aliases={"norm": normExpr}).norm[0]
        self.norm: float = xsec * lumi / weightNorm

        weightArray: ak.Array = file["Events"].arrays(["weights"], cut=cut, aliases={"weights": weightExpr}).weights
        self.normalizedWeights: ak.Array = weightArray * self.norm

    def loadExternalWeighs(self, path_to_weights: str):
        ak.from_parquet(path_to_weights)
        self.normalizedWeights *= path_to_weights

    def getWeights(self) -> ak.Array:
        return self.normalizedWeights
