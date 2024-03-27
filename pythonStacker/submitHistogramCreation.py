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
    parser.add_argument('-y', '--years', dest='years', default=["2016PreVFP", "2016PostVFP", "2017", "2018"], nargs='+',
                        help='Specific years.')


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


def args_add_toggles(parser: argparse.ArgumentParser):
    parser.add_argument('--EFT', '--eft', dest="UseEFT", default=False, action="store_true",
                        help="toggle to include EFT variations")


def arguments():
    parser = argparse.ArgumentParser(description='Your program description')

    # Add arguments
    # Adding files, reused in createHistograms
    args_add_settingfiles(parser)
    args_select_specifics(parser)
    args_add_toggles(parser)
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
