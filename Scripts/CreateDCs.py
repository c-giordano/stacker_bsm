#!/usr/bin/env python3

from PlotAllFigures import PlotFigures, PlotFiguresLocal, ParseInputArguments, GetCR, GetSettingfile
import os
import subprocess
import sys

eras = ["16PreVFP", "16PostVFP", "17", "18"]
# eras = ["16", "17", "18"]

def DCSeparateEras(inputFiles, observationFiles):
    for era in eras:
        inputFilesEra = [filename for filename in inputFiles if ("20" + era in filename) or ("Data"+era in filename)]
        obsFilesEra = [filename for filename in observationFiles if ("20" + era in filename) or ("Data"+era in filename)]
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

def LocalDatacardCreation(inputArgs):
    inputfiles, obsfiles = ParseInputArguments(inputArgs[1:])
    for era in eras:
        inputFilesEra = [filename for filename in inputfiles if ("20" + era in filename) or ("Data"+era in filename)]
        obsFilesEra = [filename for filename in obsfiles if ("20" + era in filename) or ("Data"+era in filename)]
        if (GetCR(inputFilesEra[0])):
            uncFile = "fullCR.txt"
        else:
            uncFile = "20" + era + ".txt"

        settingFile = GetSettingfile(inputFilesEra[0])
        PlotFiguresLocal(inputFilesEra, settingFile, uncFile, obsFilesEra, "-DC")

    return

if __name__ == "__main__":
    if (sys.argv[1] == "local"):
        LocalDatacardCreation(sys.argv)
    else:
        SubmitDatacardCreation(sys.argv)
