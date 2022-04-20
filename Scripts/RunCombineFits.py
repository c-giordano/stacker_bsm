#!/usr/bin/env python3

import sys
import os
import subprocess
import itertools
import json
import glob

eras = ["16PreVFP", "16PostVFP", "17", "18"]
mixTags = [["DL"], ["3L"], ["4L"], ["crw"], ["cro"], ["crz"], ["DL", "3L", "4L"], ["crw", "cro", "crz"]]
# Loop eras:
#       Loop mixTags, combine all files for era matching all tags in mixtags
# Do opposite as well: loop tags, then eras
# 

def MakeSets():
    # list all DC available
    filelist = glob.glob("/user/nivanden/Stacker_v2/combineFiles/Variations/Base/*") # should yield abs paths
    outputdatacards = []
    for era, tag in itertools.product(eras, mixTags):
        outputTag = era
        for subtag in tag:
            outputTag += "_" + subtag

        filesToCombine = []
        for file in filelist:
            if not any(subtag in file for subtag in tag) or not era in file: continue
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
        args += pureDCName + "=" + card + " "

        #outputName += "_" + pureDCName
    
    outputName += ".txt"
    
    command += args + "> " + outputDir + outputName

    os.system(command)
    return outputDir + outputName

def GetSignificance(datacards):
    with open("/user/nivanden/combineFiles/Variations/Results.json", "w") as f:
        outDict = dict()
        for datacard in datacards:
            cmd = ["Combine", "-M", "Signficance", datacard, "-t", "-1", "--expectSignal=1"]
            #os.system("Combine -M Significance " + datacard + " -t -1 --expectSignal=1")
            # pull output out of this shit
            result = subprocess.run(cmd, stdout=subprocess.PIPE)

            outDict[datacard] = result # todo: clean result
        
        jsonString = json.dumps(outDict)
        json.dump(jsonString, f)

if __name__ == "__main__":
    baseDCs = glob.glob("/user/nivanden/Stacker_v2/combineFiles/Variations/Base/*") # should yield abs paths
    combinedDCs = MakeSets()

    baseDCs.extend(combinedDCs)
    GetSignificance(baseDCs)

