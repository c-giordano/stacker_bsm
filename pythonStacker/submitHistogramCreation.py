import argparse
import json
import src.jobSubmission.condorTools as ct

def args_add_settingfiles(parser: argparse.ArgumentParser):
    # Add arguments here
    parser.add_argument('-vf', '--variablefile', dest="variablefile",
                        type=str, help='JSON file with variables.')
    parser.add_argument('-sf', '--systematicsfile', dest="systematicsfile",
                        type=str, help='JSON file with systematics.')
    parser.add_argument('-pf', '--processfile', dest='processfile',
                        type=str, help='JSON file with process definitions.')
    parser.add_argument('-cf', '--channelfile', dest='channelfile',
                        type=str, help='JSON file with channel definitions.')


def args_select_specifics(parser: argparse.ArgumentParser):
    # Add arguments here
    parser.add_argument('-v', '--variable', dest="variable", default=None,
                        type=str, help='Specific variable.')
    # Syst can be "weight", "shape", None, or a specific name
    parser.add_argument('-s', '--systematic', dest="systematic", default=None,
                        type=str, help='Specific systematic.')
    parser.add_argument('-p', '--process', dest='process', default=None,
                        type=str, help='Specific process.')
    parser.add_argument('-c', '--channel', dest='channel', default=None,
                        type=str, help='Specific channel.')


def arguments():
    parser = argparse.ArgumentParser(description='Your program description')

    # Add arguments
    # Adding files, reused in createHistograms
    args_add_settingfiles(parser)
    args_select_specifics(parser)
    # Adding further selections:

    args = parser.parse_args()
    return args


def get_keys(file):
    with open(file, 'r') as f:
        keys = list(json.load(f).keys())
    return keys


if __name__ == "__main__":
    args = arguments()

    basecommand = "python3 createHistograms.py"
    basecommand += f" --variablefile {args.variablefile}"
    basecommand += f" --processfile {args.processfile}"
    basecommand += f" --systematicsfile {args.systematicsfile}"
    basecommand += f" --channelfile {args.channelfile}"

    processlist = get_keys(args.processfile)
    channellist = get_keys(args.channelfile)

    commandset = []
    for process in processlist:
        if process != args.process:
            continue

        commandset_process = []

        for channel in channellist:
            if args.channel is not None and channel != args.channel:
                continue
            # check if channel is subchannel! Otherwise no plot to be made.
            # can just add the command and not care

            command = basecommand
            command += f" --process {process}"
            command += f" --channel {channel}"

            if args.variable is not None:
                command += f" --variable {args.variable}"
            if args.systematic is not None:
                command += f" --systematic {args.systematic}"

            commandset_process.append(command)

        commandset.append(commandset_process)

    ct.submitCommandsetsAsCondorCluster("CreateHistograms", commandset)
    # submit commandset
