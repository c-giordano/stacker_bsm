from src.configuration.Uncertainty import Uncertainty


def generate_scalevariations(dict_entry):
    ret = [ScaleVariation("ScaleVar", dict_entry, i) for i in range(6)]
    return ret


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
