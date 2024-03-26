"""
For files with eft weights, prepare and store the weights per event class, since it takes a lot of computing time.
"""

from plugins.eft import eft_reweighter
import awkward as ak
import uproot
import argparse
import os

from createHistograms import get_tree_from_file


def arguments():
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument("-f", "--file", dest="inputfile", action="store", required=True,
                        help="Inputfile for which variations must be stored")
    parser.add_argument("-e", "--eventClass", dest="eventclass", action="store", default=11,
                        help="which event class to check")
    parser.add_argument("--storage", dest="storage", type=str,
                        default="Intermediate", help="Path at which the histograms are stored")

    opts, opts_unknown = parser.parse_known_args()
    return opts


def get_eftvariations_filename(storage: str, filename: str, eventclass) -> str:
    if not os.path.exists(os.path.join(storage, "eftWeights")):
        os.makedirs(os.path.join(storage, "eftWeights"))
    return os.path.join(storage, "eftWeights", "evClass_" + str(eventclass) + "_wgts.parquet")


def reweight_and_write(reweighter, eventclass, tree, storage):
    arrs = tree.arrays(["eftVariationsNorm"], "eventClass==" + str(eventclass), aliases={"eftVariationsNorm": "eftVariations/eftVariations[:,0]"})

    # should become a record rather than a single array, using the naming scheme I devised in the init

    var = ak.Array(reweighter.transform_weights(entry[1:]) for entry in arrs.eftVariationsNorm)

    if len(var) == 0:
        return

    outputfile = get_eftvariations_filename(storage, "", eventclass)
    ak.to_parquet(var, outputfile)
    return


if __name__ == "__main__":
    args = arguments()

    reweighter = eft_reweighter()
    # tree = uproot.open(args.inputfile)

    tree = get_tree_from_file(args.inputfile, "TTTT_EFT")
    if (int(args.eventclass) == -1):
        for i in range(15):
            reweight_and_write(reweighter, i, tree, args.storage)
    else:
        print(f"running weights for class {args.eventclass}")
        reweight_and_write(reweighter, int(args.eventclass), tree, args.storage)
