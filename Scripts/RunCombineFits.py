#!/usr/bin/env python3

import sys
import os
import subprocess
import itertools
import json
import glob

eras = [["16PreVFP"], ["16PostVFP"], ["17"], ["18"], ["16PreVFP", "16PostVFP"], ["16PreVFP", "16PostVFP", "17", "18"]]
mixTags = [["DL"], ["_3L"], ["_4L"], ["crw"], ["cro"], ["crz"], ["crz-4L"], ["cro-3L"], ["DL", "_3L", "_4L"], ["crw", "cro", "crz", "cro-3L", "crz-4L"], ["DL", "_3L", "_4L", "crw", "cro", "crz", "cro-3L", "crz-4L"]]
# Loop eras:
#       Loop mixTags, combine all files for era matching all tags in mixtags
# Do opposite as well: loop tags, then eras
# 

def MakeSets():
    # list all DC available
    filelist = glob.glob("/user/nivanden/Stacker_v2/combineFiles/Variations/Base/*") # should yield abs paths
    outputdatacards = []
    for era, tag in itertools.product(eras, mixTags):
        outputTag = ""
        for subera in era:
            outputTag += subera + "_"
        for subtag in tag:
            outputTag += subtag + "_" 
        
        outputTag = outputTag[:-1]

        filesToCombine = []
        for file in filelist:
            if not any(subtag in file for subtag in tag) or not any(subera in file for subera in era): continue
            filesToCombine.append(file)
        
        newDatacard = CombineDatacards(filesToCombine, outputTag)
        outputdatacards.append(newDatacard)

    return outputdatacards

def CombineDatacards(listOfCards, outputTag):
    # define name based on DC name
    # next combine all the shit
    # define outputname independently or based on inputs...
    command = "combineCards.py "
    args = ""
    outputName = "DC_Combi_" + outputTag
    outputDir = "/user/nivanden/Stacker_v2/combineFiles/Variations/Combinations/"
    for card in listOfCards:
        pureDCName = card.split('/')[-1].split(".")[0][3:]
        args += "y" + pureDCName + "=" + card + " "

        #outputName += "_" + pureDCName
    
    outputName += ".txt"
    
    command += args + "> " + outputDir + outputName

    os.system(command)
    return outputDir + outputName

def GetSignificance(datacards):
    outDict = dict()
    for datacard in datacards:
        cmd = ["combine", "-M", "Significance", datacard, "-t", "-1", "--expectSignal=1"]
        #os.system("Combine -M Significance " + datacard + " -t -1 --expectSignal=1")
        # pull output out of this shit
        result = subprocess.run(cmd, stdout=subprocess.PIPE)
        resultString =  result.stdout.decode('utf-8').split('\n')[-3:]
        resultString = "\n".join(resultString)

        outDict[datacard] = resultString # todo: clean result
        
    with open("/user/nivanden/Stacker_v2/combineFiles/Variations/Results.json", "w") as f:
        jsonString = json.dumps(outDict)
        json.dump(outDict, f)

if __name__ == "__main__":
    baseDCs = glob.glob("/user/nivanden/Stacker_v2/combineFiles/Variations/Base/*") # should yield abs paths
    combinedDCs = MakeSets()

    baseDCs.extend(combinedDCs)
    GetSignificance(baseDCs)

