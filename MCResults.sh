#!/bin/bash

# combine datacards
file_16Pre=$(ls -t ../ewkino/_FourTopAnalysis/Output/*2016Pre* | head -1)
file_16Post=$(ls -t ../ewkino/_FourTopAnalysis/Output/*2016Post* | head -1)
file_17=$(ls -t ../ewkino/_FourTopAnalysis/Output/*2017* | head -1)
file_18=$(ls -t ../ewkino/_FourTopAnalysis/Output/*2018* | head -1)

./stacker_exec $file_16Pre $file_16Post $file_17 $file_18 SettingFiles/main.txt -unc UncertaintyFiles/full.txt
./stacker_exec $file_16Pre $file_16Post $file_17 $file_18 SettingFiles/main.txt -unc UncertaintyFiles/full.txt -IP

./stacker_exec $file_16Pre SettingFiles/main.txt -unc UncertaintyFiles/2016PreVFP.txt -DC
./stacker_exec $file_16Post SettingFiles/main.txt -unc UncertaintyFiles/2016PostVFP.txt -DC
./stacker_exec $file_17 SettingFiles/main.txt -unc UncertaintyFiles/2017.txt -DC
./stacker_exec $file_18 SettingFiles/main.txt -unc UncertaintyFiles/2018.txt -DC

cd combineFiles
combineCards.py y16Pre=DC_2016PreVFP.txt y16Post=DC_2016PostVFP.txt y17=DC_2017.txt y18=DC_2018.txt > combinedDatacard.txt

combine -M Significance combinedDatacard.txt -t -1 --expectSignal=1
