#!/usr/bin/env python3

from PlotAllFigures import PlotFigures, ParseInputArguments, GetCR, GetSettingfile
import sys

eras = ["16PreVFP", "16PostVFP", "17", "18"]

def DCSeparateEras(inputFiles, observationFiles):
    for era in eras:
        inputFilesEra = [filename for filename in inputFiles if era in filename]
        obsFilesEra = [filename for filename in observationFiles if era in filename]
        if (GetCR(inputFilesEra[0])):
            uncFile = "fullCR.txt"
        else:
            uncFile = "20" + era + ".txt"

        settingFile = GetSettingfile(inputFilesEra[0])
        PlotFigures(inputFilesEra, settingFile, uncFile, obsFilesEra, "-DC")

    return

def SubmitDatacardCreation(inputArgs):
    inputfiles, obsfiles = ParseInputArguments(inputArgs)
    DCSeparateEras(inputfiles, obsfiles)

if __name__ == "__main__":
    SubmitDatacardCreation(sys.argv)
