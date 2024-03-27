import argparse
import json
import src.jobSubmission.condorTools as ct

import src.arguments as arguments


def parse_arguments():
    parser = argparse.ArgumentParser(description='Your program description')

    # Add arguments
    # Adding files, reused in createHistograms
    arguments.add_settingfiles(parser)
    arguments.select_specifics(parser)
    arguments.add_toggles(parser)
    # Adding further selections:

    args = parser.parse_args()
    return args


def get_keys(file):
    with open(file, 'r') as f:
        keys = list(json.load(f).keys())
    return keys


if __name__ == "__main__":
    args = parse_arguments()

    basecommand = "python3 createHistograms.py"
    basecommand += f" --variablefile {args.variablefile}"
    basecommand += f" --processfile {args.processfile}"
    basecommand += f" --systematicsfile {args.systematicsfile}"
    basecommand += f" --channelfile {args.channelfile}"

    with open(args.processfile, 'r') as f:
        processlist = list(json.load(f)["Processes"].keys())

    with open(args.channelfile, 'r') as f:
        channels = json.load(f)
        channellist = list(channels.keys())

    commandset = []
    for year in args.years:
        for process in processlist:
            if args.process is not None and process != args.process:
                continue

            commandset_process = []

            for channel in channellist:
                if args.channel is not None and channel != args.channel:
                    continue
                if channels[channel].get("isSubchannel", 0) > 0:
                    continue
                # check if channel is subchannel! Otherwise no plot to be made.
                # can just add the command and not care

                command = basecommand
                command += f" --process {process}"
                command += f" --channel {channel}"
                command += f" -y {year}"

                if args.variable is not None:
                    command += f" --variable {args.variable}"
                if args.systematic is not None:
                    command += f" --systematic {args.systematic}"
                else:
                    command += " -s weight"
                if args.UseEFT is True:
                    command += " --EFT"

                commandset_process.append(command)

            commandset.append(commandset_process)

    ct.submitCommandsetsAsCondorCluster("CreateHistograms", commandset, scriptfolder="Scripts/condor/")
    # submit commandset
