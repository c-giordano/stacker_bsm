import json

from src.configuration.Uncertainty import Uncertainty


def load_uncertainties(jsonfile, typefilter=None):
    with open(jsonfile, 'r') as f:
        all_data = json.load(f)
    ret = []
    for key, val in all_data.items():
        if typefilter and val.get("type", "flat") != typefilter:
            continue
        ret.append(Uncertainty(key, val))
    return ret


class Channel:
    def __init__(self) -> None:
        self.processes = []

    def is_process_included(self, process: str):
        return process in self.processes


# might replace with a class just keeping track of a few arrays.
# TODO: start from json file or at least dict
class Process:
    def __init__(self) -> None:
        self._isSignal = False
        self.name = ""

    @property
    def isSignal(self):
        return self._isSignal

    @isSignal.setter
    def isSignal(self, value):
        if not isinstance(value, bool):
            raise ValueError("isSignal must be a boolean value")
        self._isSignal = value
