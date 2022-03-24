#!/bin/bash

# combine datacards

eras=("16PreVFP" "16PostVFP" "17" "18")
erasAlt=("2016" "2017" "2018")
# make  sure this stays the same or find a better solution
process=("TTTT" "TTZ" "TTW" "TTH" "ttbar" "Xgamma" "Rare_0" "Rare_1" "Rare_2" "Rare_3" "Rare_4" "Rare_5" "TTVV_0" "TTVV_1") 

filestring=""

for e in ${eras[@]}; do
    for p in ${process[@]}; do
        file=$(ls -t ../ewkino/_FourTopAnalysis/Output/*MCAll*CR*$e*$p* | head -1)
        filestring=${filestring}$file
        filestring=${filestring}" "
    done
done

datastring=""
for e in ${eras[@]}; do
    file=$(ls -t ../ewkino/_FourTopAnalysis/Output/*Obs*CR*$e* | head -1)
    datastring=${datastring}$file
    datastring=${datastring}" "
done


echo $filestring
echo $datastring

./stacker_exec $filestring SettingFiles/main.txt -unc UncertaintyFiles/full.txt -RD $datastring
