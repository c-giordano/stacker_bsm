# TODO: add support for processes flags such as "MC"/"AllMC"/...

class Uncertainty:
    def __init__(self, name, dict_entry):
        self.name = name
        # extract dictionary and add defaults for every property
        self.pretty_name = dict_entry.get("pretty_name", name)
        self.channels = dict_entry.get("channels", ["all"])
        self.processes = dict_entry.get("processes", ["all"])
        self._correlated_process = bool(dict_entry.get("corr_proc", True))

        # Possible types: flat, weight, shape
        self.type = dict_entry.get("type", "flat")
        if self.type == "flat":
            self._isFlat = False

        self.rate = dict_entry.get("rate", 1.)

    @property
    def isFlat(self):
        return self._isFlat

    @isFlat.setter
    def isFlat(self, value):
        if isinstance(value, bool):
            self._isFlat = value
        else:
            raise ValueError("isFlat must be a boolean")

    @property
    def correlated_process(self):
        return self._correlated_process

    @correlated_process.setter
    def correlated_process(self, value):
        if isinstance(value, bool):
            self._correlated_process = value
        else:
            raise ValueError("correlated_process must be a boolean")

    def is_channel_relevant(self, channel):
        if self.channels[0] == "all":
            return True
        return channel in self.channels

    def is_process_relevant(self, process):
        if self.processes[0] == "all":
            return True
        return process in self.processes
