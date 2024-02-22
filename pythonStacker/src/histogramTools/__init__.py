# import numpy as np
import awkward as ak
import os
from typing import Union


class HistogramManager:
    def __init__(self, storagepath: str, process: str, var_names: list[str], systematics: Union[list[str], None] = None):
        self.store: str = storagepath
        self.process: str = process
        self.variables: list = var_names

        self.systematics = ["nominal"]
        if self.systematics is list:
            self.systematics.extend(systematics)

        filename = process.replace(" ", "_") + "_" # + sys + ".parquet"
        self.cache_folder = dict()
        self.base_name = dict()
        for var in self.variables:
            self.cache_folder[var] = os.path.join(storagepath, var)
            self.base_name[var] = os.path.join(self.cache_folder[var], filename)

    def get_name(self, var: str, sys: str = "nominal"):
        return self.base_name[var] + sys + ".parquet"

    def load_all_histograms(self):
        ret = dict()
        for var in self.variables:
            ret[var] = self.load_histograms_for_variable(var)
        return ret

    def load_histograms_for_variable(self, var):
        ret = dict()
        for sys in self.systematics:
            ret[sys] = self.load_histogram(var, sys)

        return ret

    def load_histogram(self, var: str, sys: str = "nominal"):
        content = ak.to_numpy(ak.from_parquet(self.get_name(var, sys)))
        return content

    def save_all_histograms(self, content):
        for var in self.variables:
            self.save_histograms_for_variable(content[var], var)

    def save_histograms_for_variable(self, content_variations, var):
        for sys in self.systematics:
            self.save_histogram(content_variations[sys], var, sys)

    def save_histogram(self, content, var: str, sys: str = "nominal"):
        ak.to_parquet(content, self.get_name(var, sys))
