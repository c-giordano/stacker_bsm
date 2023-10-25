"""
For files with eft weights, prepare and store the weights per event class, since it takes a lot of computing time.
"""

from plugins.eft import eft_reweighter
import awkward as ak
import uproot
import argparse

def get_eftvariations_filename(filename: str, eventclass) -> str:
    return "output/eftWeights/evClass_"+str(eventclass)+"_wgts.parquet"

def reweight_and_write(reweighter, eventclass, tree):
    arrs = tree.arrays(["eftVariationsNorm"], "eventClass=="+str(eventclass), aliases={"eftVariationsNorm" : "eftVariations/eftVariations[:,0]"})
    
    var = ak.Array(reweighter.transform_weights(entry[1:]) for entry in arrs.eftVariationsNorm)

    if len(var) == 0:
        return
    
    outputfile = get_eftvariations_filename("", eventclass)
    ak.to_parquet(var, outputfile)
    return


if __name__ == "__main__":    
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument("-f", "--file", dest="inputfile", action="store", required=True,
                        help="Inputfile for which variations must be stored")
    parser.add_argument("-e", "--eventClass", dest="eventclass", action="store", default=-1,
                        help="which event class to check")

    opts, opts_unknown = parser.parse_known_args()

    reweighter = eft_reweighter()
    tree: uproot.TTree = uproot.open(opts.inputfile+":test")

    if (int(opts.eventclass) == -1):
        for i in range(15):
            reweight_and_write(reweighter, i, tree)
    else:
        print(f"running weights for class {opts.eventclass}")
        reweight_and_write(reweighter, int(opts.eventclass), tree)

