import argparse
import src.jobSubmission.condorTools as ct

from submitHistogramCreation import args_add_settingfiles, args_select_specifics


def arguments():
    parser = argparse.ArgumentParser(description='Script to submit plotting of histograms')
    args_add_settingfiles(parser)
    args_select_specifics(parser)

    args = parser.parse_args()
    return args


if __name__ == "__main__":
    args = arguments()

    basecommand = "python3 createSystematicBand.py"
    basecommand += f" --variablefile {args.variablefile}"
    basecommand += f" --processfile {args.processfile}"
    basecommand += f" --systematicsfile {args.systematicsfile}"
    basecommand += f" --channelfile {args.channelfile}"

    cmds = []
    for year in args.years:
        cmd = basecommand + f" -y {year}"
        cmds.append([cmd])

    if "2016PreVFP" in args.years and "2016PostVFP" in args.years:
        cmd = basecommand + " -y 2016PreVFP 2016PostVFP"
        cmds.append([cmd])

    if len(args.years) == 4:
        cmd = basecommand + " -y 2016PreVFP 2016PostVFP 2017 2018"
        cmds.append([cmd])

    ct.submitCommandsetsAsCondorCluster("SystematicsBand", cmd, scriptfolder="Scripts/condor/")
