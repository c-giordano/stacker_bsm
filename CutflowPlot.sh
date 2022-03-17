#!/bin/bash

# combine datacards

eras=("2016PreVFP" "2016PostVFP" "2017" "2018")
process=("TTTT" "TTZ" "TTW" "TTH" "ttbar" "Xgamma" "Rare" "TTVV") 

filestring=""

for e in ${eras[@]}; do
    for p in ${process[@]}; do
        file=$(ls -t ../ewkino/_FourTopAnalysis/Output/Cutflow*$e*$p* | head -1)
        filestring=${filestring}$file
        filestring=${filestring}" "
    done
done

echo $filestring

./stacker_exec $filestring SettingFiles/cutflow.txt
