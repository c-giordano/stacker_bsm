import glob
import subprocess
import os
import sys

sys.path.append(os.path.abspath('../'))

import ExecuteStackerOnCondor

# make sure cwd is stacker home dir

def SubmitIPJob(datacard):
    cmd = "python3 Scripts/GenerateImpactPlotsPerDC.py " + datacard
    ExecuteStackerOnCondor.submitCommandAsCondorJob("IPGeneration", cmd)
    return

if __name__ == "__main__":
    os.chdir('/user/nivanden/Stacker_v2/')

    baseDCs = glob.glob("/user/nivanden/Stacker_v2/combineFiles/Variations/Base/*") # should yield abs paths
    combinedDCs = glob.glob("/user/nivanden/Stacker_v2/combineFiles/Variations/Combinations/*")

    baseDCs.extend(combinedDCs)

    for dc in baseDCs:
        SubmitIPJob(dc)
