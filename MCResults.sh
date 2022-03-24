#!/bin/bash

# combine datacards

eras=("2016PreVFP" "2016PostVFP" "2017" "2018")
erasAlt=("2016" "2017" "2018")
# make  sure this stays the same or find a better solution
process=("TTTT" "TTZ" "TTW" "TTH" "ttbar" "Xgamma" "Rare_0" "Rare_1" "Rare_2" "Rare_3" "Rare_4" "Rare_5" "TTVV_0" "TTVV_1") 

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
    echo $filestring
    ./stacker_exec $filestring SettingFiles/main.txt -unc UncertaintyFiles/$e.txt -DC
done

cd combineFiles
combineCards.py y16Pre=DC_2016PreVFP.txt y16Post=DC_2016PostVFP.txt y17=DC_2017.txt y18=DC_2018.txt > combinedDatacard.txt
combine -M Significance combinedDatacard.txt -t -1 --expectSignal=1
