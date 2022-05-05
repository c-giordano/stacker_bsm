import subprocess
import sys
import os
import shutil

baseDir = "/user/nivanden/Stacker_v2/combineFiles/Variations/Impacts/"

def DoTextToWorkspace(datacard):
    rawDCName = dc.split("/")[-1]
    rawDCTag = rawDCName[3:-4]
    
    shutil.rmtree(baseDir+rawDCTag, ignore_errors=True) 
    
    if not os.path.exists(baseDir+rawDCTag):
        os.mkdir(baseDir+rawDCTag)
    
    os.chdir(baseDir+rawDCTag)

    cmd = ["text2workspace.py", datacard, "-m", "125"]
    result = subprocess.run(cmd, stdout=subprocess.PIPE)
    
    os.replace(datacard.replace(".txt", ".root"), baseDir + rawDCTag + "/" + rawDCName.replace(".txt", ".root"))
    return baseDir + rawDCTag + "/" + rawDCName.replace(".txt", ".root")

def GenerateInitialFit(datacard):
    cmd = ["combineTool.py", "-M", "Impacts", "-d", datacard, "-m", "125", "--doInitialFit", "--robustFit", "1", "--expectSignal=1"]
    
    result = subprocess.run(cmd, stdout=subprocess.PIPE)
    return

def CreateIPFits(datacard):
    cmd = ["combineTool.py", "-M", "Impacts", "-d", datacard, "-m", "125", "--robustFit", "1", "--doFits", "--expectSignal=1"]
    
    result = subprocess.run(cmd, stdout=subprocess.PIPE)
    return

def PostProcess(datacard):
    cmd1 = ["combineTool.py", "-M", "Impacts", "-d", datacard, "-m", "125", "--o", "impacts.json"]
    cmd2 = ["plotImpacts.py", "-i", "impacts.json", "-o", "impacts"]
    
    result = subprocess.run(cmd1, stdout=subprocess.PIPE)
    result = subprocess.run(cmd2, stdout=subprocess.PIPE)
    return

if __name__ == "__main__":
    dc = sys.argv[1]
    
    workspaceFile = DoTextToWorkspace(dc)
    GenerateInitialFit(workspaceFile)
    CreateIPFits(workspaceFile)
    PostProcess(workspaceFile)

    # make dir in variations 

    # do text 2 workspace
    # whole IP stuff
    