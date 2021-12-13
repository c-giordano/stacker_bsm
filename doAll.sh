#!/bin/bash

# hadd files -> todo
# other stuff
# en generalize this

# combine datacards
file_16Pre=$(ls -t ../ewkino/_FourTopAnalysis/Output/*2016Pre* | head -1)
file_16Post=$(ls -t ../ewkino/_FourTopAnalysis/Output/*2016Post* | head -1)
file_17=$(ls -t ../ewkino/_FourTopAnalysis/Output/*2017* | head -1)
file_18=$(ls -t ../ewkino/_FourTopAnalysis/Output/*2018* | head -1)

./stacker_exec $file_16Pre SettingFiles/main.txt -unc UncertaintyFiles/2016PreVFP.txt -DC
./stacker_exec $file_16Post SettingFiles/main.txt -unc UncertaintyFiles/2016PostVFP.txt -DC
./stacker_exec $file_17 SettingFiles/main.txt -unc UncertaintyFiles/2017.txt -DC
./stacker_exec $file_18 SettingFiles/main.txt -unc UncertaintyFiles/2018.txt -DC

combineCards.py y16Pre=2016PreVFP.txt y16Post=2016PostVFP.txt y17=2017.txt y18=2018.txt > combineFiles/combinedDatacard.txt

#./stacker_exec AnalysisOutput/AnalysisOutput_13_12_2021-11_All.root SettingFiles/main.txt -unc UncertaintyFiles/full.txt -DC
