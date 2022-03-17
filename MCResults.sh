#!/bin/bash

# combine datacards

eras=("2016PreVFP" "2016PostVFP" "2017" "2018")
process=("TTTT" "TTZ" "TTW" "TTH" "ttbar" "Xgamma" "Rare" "TTVV") 

filestring=""

for e in ${eras[@]}; do
    for p in ${process[@]}; do
        file=$(ls -t ../ewkino/_FourTopAnalysis/Output/*MCAll*$e*$p* | head -1)
        filestring=${filestring}$file
        filestring=${filestring}" "
    done
done

echo $filestring

./stacker_exec $filestring SettingFiles/main.txt -unc UncertaintyFiles/full.txt

for e in ${eras[@]}; do
    filestring=""
    for p in ${process[@]}; do
        file=$(ls -t ../ewkino/_FourTopAnalysis/Output/*MCAll*$e*$p* | head -1)
        filestring=${filestring}$file
        filestring=${filestring}" "
    done
    
    ./stacker_exec $filestring SettingFiles/main.txt -unc UncertaintyFiles/$e.txt -DC
done

cd combineFiles
combineCards.py y16Pre=DC_2016PreVFP.txt y16Post=DC_2016PostVFP.txt y17=DC_2017.txt y18=DC_2018.txt > combinedDatacard.txt
combine -M Significance combinedDatacard.txt -t -1 --expectSignal=1
