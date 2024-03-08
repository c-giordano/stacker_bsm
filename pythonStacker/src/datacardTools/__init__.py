from src.configuration.Uncertainty import Uncertainty


class DatacardWriter():
    def __init__(self, filename) -> None:
        self.file = filename
        self.outputstring = ""
        self.channels = []
        self.processes = []

    def commentline(self, length=200):
        self.outputstring += "-" * length + "\n"

    def initialize_datacard(self, number_of_regions, rootfile):
        self.outputstring = f"imax {number_of_regions}\n"
        self.outputstring += "jmax *\n"
        self.outputstring += "kmax *\n"
        self.commentline()
        self.outputstring += f"shapes * * {rootfile} $CHANNEL/$PROCESS $CHANNEL/$SYSTEMATIC/$PROCESS\n"
        self.commentline()

    def add_channels(self, pretty_names_channels, channels):
        self.channels = channels
        self.channel_names = pretty_names_channels
        self.outputstring += "{:>25s}".format("bin")
        for channel in self.channel_names:
            self.outputstring += f"\t{channel:>15s}"
        self.outputstring += "\n"
        self.outputstring += "{:>25s}".format("observation")
        observationline = f"\t{-1:>15d}" * len(channels)
        self.outputstring += observationline
        self.outputstring += "\n"
        self.commentline()

    def add_processes(self, processes: list):
        if len(self.channels) == 0:
            raise Exception("No channels defined! Run add_channels before add_processes!")

        channelline = ""
        processline = ""
        processnumber = ""
        rate = ""
        self.processes = processes
        # process should be an object itself containing a lot of information
        # same for channel -> not super lightweight but ok
        # need to refix this
        for channelname, channel in zip(self.channel_names, self.channels):
            for process in self.processes:
                if channel.is_process_excluded(process):
                    continue

                channelline += "{:>15s}\t".format(channelname)
                processline += "{:>15s}\t".format(process)
                processnumber += "{:>15d}\t".format(0)
                rate += "{:>15d}\t".format(-1)

        self.outputstring += "{:>25s}\t".format("bin") + channelline + "\n"
        self.outputstring += "{:>25s}\t".format("process") + processline + "\n"
        self.outputstring += "{:>25s}\t".format("process") + processnumber + "\n"
        self.outputstring += "{:>25s}\t".format("rate") + rate + "\n"
        self.commentline()

    def add_MCstats(self):
        self.commentline()
        self.outputstring += "* autoMCStats 0 1 1\n"

    def add_systematic(self, systematic: Uncertainty):
        # similar to add process stuff
        systematic_line = ""
        relevant = False
        for channel in self.channels:
            if not systematic.is_channel_relevant(channel):
                continue
            for process in self.processes:
                if systematic.is_process_relevant(process):
                    systematic_line += "\t{:>15.2f}".format(systematic.rate)
                    relevant = True
                else:
                    systematic_line += "\t{:>15s}".format("-")

        if relevant:
            self.outputstring += "{:<18s} ".format(systematic.name)
            if systematic.isFlat:
                self.outputstring += "{:>6s}".format("lnN")
            else:
                self.outputstring += "{:>6s}".format("shape")

            self.outputstring += systematic_line + "\n"

    def write_card(self):
        # write autoMCStats line
        self.add_MCstats()
        with open(self.file, "w") as f:
            f.write(self.outputstring)


if __name__ == "__main__":
    def test_DatacardWrite():
        # Create a DatacardWrite object
        datacard = DatacardWriter('/home/njovdnbo/Documents/Stacker_v2/pythonStacker/output/test/test.txt')

        # Initialize the datacard with 3 regions
        datacard.initialize_datacard(3, "blub.root")

        # Add a comment line
        datacard.add_channels(["pretty"], ["BDT_EM_region"])
        datacard.add_processes(["blub"])

        # Write the datacard to the file
        datacard.write_card()

    # Run the test
    test_DatacardWrite()
