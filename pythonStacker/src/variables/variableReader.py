import json
import os
from src.variables import get_method_from_str


class Variable:
    def __init__(self, name, data):
        self.name = name
        # Load in the information stored for the key in data
        self.range = data.get('range', [0, 10])
        self.nbins = data.get('nbins', 10)
        self.method_name = data.get("method_name", "")
        self.branch_name = data.get("branch_name", "")
        self.axis_label = data.get("axis_label", "")

        # Add more attributes as needed, with default values
        self.channels = data.get("channels", ["all"])
        if type(self.channels) is not list:
            self.channels = [self.channels]

    def get_method(self):
        return get_method_from_str(self.method_name)

    def is_channel_relevant(self, channel):
        if self.channels[0] == "all":
            return True
        if any([channel in option for option in self.channels]) or any([option in channel for option in self.channels]):
            return True
        return False


class VariableReader:
    def __init__(self, filename: str, variables: list, channel=""):
        self.filename = filename
        # load file:
        with open(self.filename, 'r') as file:
            data: dict = json.load(file)

        extra_files = data.get("load_variables", [])
        for extra_file in extra_files:
            with open(os.path.join("settingfiles/Variables", extra_file), 'r') as file:
                extra_data = json.load(file)
            data.update(extra_data)
        print(data)
        if variables is None or variables == "all":
            self.variables = list(data.keys())
        else:
            self.variables = variables
        if type(self.variables) is not list:
            self.variables = [self.variables]

        self.nvar = len(self.variables)
        self.variable_objects: dict[str, Variable] = {}

        variables_subset = []
        for key in self.variables:
            if key == "load_variables":
                continue
            new_variable = Variable(key, data[key])
            if not new_variable.is_channel_relevant(channel):
                continue
            self.variable_objects[key] = new_variable
            variables_subset.append(key)
        self.variables = variables_subset
        self.index = 0

    def __getitem__(self, key) -> Variable:
        return self.variable_objects[key]

    def __setitem__(self, key, value):
        self.variable_objects[key] = value

    def get_properties(self, variable) -> Variable:
        return self.variable_objects[variable]

    def get_variables(self) -> list[str]:
        return self.variables

    def get_variable_objects(self):
        return self.variable_objects

    def number_of_variables(self):
        return self.nvar

    def __iter__(self):
        return self

    def __next__(self):
        if self.index < len(self.variables):
            result = self.variable_objects[self.variables[self.index]]
            self.index += 1
            return result
        else:
            raise StopIteration
