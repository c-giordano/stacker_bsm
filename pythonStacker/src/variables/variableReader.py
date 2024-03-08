import json
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

    def get_method(self):
        return get_method_from_str(self.method_name)


class VariableReader:
    def __init__(self, filename: str, variables: list):
        self.filename = filename
        # load file:
        with open(self.filename, 'r') as file:
            data = json.load(file)

        if variables is None or variables == "all":
            self.variables = list(data.keys())
        else:
            self.variables = variables

        self.nvar = len(self.variables)
        self.variable_objects: dict[str, Variable] = {}

        for key in self.variables:
            self.variable_objects[key] = Variable(key, data[key])
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
