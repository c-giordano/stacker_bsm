import awkward as ak
import uproot
import numpy as np


def load_variable(tree: uproot.TTree, branchname: str, selection: str):
    arr = tree.arrays(["variable"], cut=selection, aliases={"variable": branchname})
    return ak.to_numpy(arr.variable)


def bdt_tanh_transfrom(tree: uproot.TTree, branchname: str, selection: str):
    """
    Note: variable should be treatable as a regular-shaped numpy-like array (ideally)
    """
    variable = tree.arrays(["variable"], cut=selection, aliases={"variable": branchname}).variable

    # variable = ak.where(variable > 1., ak.full_like(variable, 0.9999), variable)
    old_shape = ak.num(variable)
    flattened = ak.to_numpy(ak.flatten(variable))
    flattened[flattened > 1.] = 0.9999
    transformed = np.arctanh((flattened + 1.) * 0.5)
    unflattened = ak.unflatten(transformed, old_shape)

    return unflattened


def sum_branch(tree: uproot.TTree, branchname: str, selection: str):
    variable = tree.arrays(["variable"], cut=selection, aliases={"variable": branchname}).variable
    return ak.to_numpy(ak.sum(variable, axis=1))


def get_method_from_str(method: str):
    tmp = {
        "load": load_variable,
        "default": load_variable,
        "tanh": bdt_tanh_transfrom,
        "sum_branch": sum_branch
    }
    return tmp[method]
