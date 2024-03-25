import argparse
import os
import json
import glob
import src.jobSubmission.condorTools as ct

from submitHistogramCreation import args_add_settingfiles


def arguments():
    parser = argparse.ArgumentParser(description=__doc__)

    args_add_settingfiles(parser)
    args, args_unknown = parser.parse_known_args()
    return args


if __name__ == "__main__":
    args = arguments()

    # loop processes and check which one has EFT, then submit it
    with open(args.processfile, 'r') as f:
        content = json.load(f)
        processes = content["processes"]
        basedir = content["basedir"]

    commandset = []
    for process in processes:
        # if has EFT
        files = []
        for filebase in process["fileglobs"]:
            fileglob = os.path.join(basedir, filebase)
            fileglob += f"*{args.years[0]}"
            if args.systematic == "weight" or args.systematic is None:
                fileglob += "*base.root"
            true_files = glob.glob(fileglob)
            files.extend(true_files)

        cmds = []
        for filename in files:
            if process.get("hasEFT", 0) == 0:
                continue
            for eventclass in range(14):
                cmd = "python3 createEFTWeights.py"
                cmd += f" -f {filename}"
                cmd += f" -e {eventclass}"
                cmds.append(cmd)

        commandset.append(cmds)

    ct.submitCommandsetsAsCondorCluster("CreateEFTWeights", commandset, scriptfolder="Scripts/condor/")
