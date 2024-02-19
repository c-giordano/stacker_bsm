import json


class Config:
    def __init__(self, filename):
        self.filename = filename
        self.contents = self.read_file()

    def read_file(self):
        with open(self.filename, 'r') as file:
            return json.load(file)


class UncertaintyConfig(Config):
    def __init__(self, filename):
        super().__init__(filename)


class ProcessConfig(Config):
    def __init__(self, filename):
        super().__init__(filename)


class PlottingConfig(Config):
    def __init__(self, filename):
        super().__init__(filename)
