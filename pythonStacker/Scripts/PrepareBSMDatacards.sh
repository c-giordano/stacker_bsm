#!/bin/bash
# Script to generate datacards for each BSM model

source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /user/nivanden/CMSSW_13_3_3/src
eval `scram runtime -sh`
export X509_USER_PROXY=/user/$USER/x509up_u$(id -u $USER)

cd /user/nivanden/plots/pythonStacker/

CreateProcessFile () {
    cp "settingfiles/Process/BSM_template.json" "settingfiles/Process/bsm_var/${1}.json"
    # replace BSMMODEL with the proper bsmmodel name,
    # replace BSMNAME with the pretty name for this model
    sed -i "s/BSMMODEL/${1}/g" "settingfiles/Process/bsm_var/${1}.json"
    sed -i "s/BSMNAME/${2}/g" "settingfiles/Process/bsm_var/${1}.json"
}


RunDatacardCreation () {
    # args: Base model, Mass point, Year
    BSMMODEL="TopPhilic${1}_M${2}"
    CreateProcessFile $BSMMODEL $3
    python3 buildDatacard.py -vf settingfiles/Variables/base.json \
                             -sf settingfiles/Uncertainties/${3}_light.json \
                             -pf settingfiles/Process/bsm_var/${BSMMODEL}.json \
                             -cf settingfiles/Channel/all_channels.json \
                             -y ${3} \
                             -dcf settingfiles/Datacards/${3}_full.json \
                             -op output/datacards/tmp/
    # move the datacard to the proper directory
    mkdir -p "output/bsmlimits/TopPhilic${1}/${4}"
    mv "output/datacards/tmp/DC_${3}.txt" "output/bsmlimits/${1}/${4}/"
    mv "output/datacards/tmp/DC_${3}.root" "output/bsmlimits/${1}/${4}/"
}

# check if the directory bsm_var exists:
if [ ! -d "settingfiles/Process/bsm_var" ]
then
    mkdir "settingfiles/Process/bsm_var"
fi

# Build directory structure
if [ ! -d "output/bsmlimits" ]
then
    mkdir "output/bsmlimits"
fi
if [ ! -d "output/bsmlimits/TopPhilicScalarSinglet" ]
then
    mkdir "output/bsmlimits/TopPhilicScalarSinglet"
fi
if [ ! -d "output/bsmlimits/TopPhilicPseudoScalarSinglet" ]
then
    mkdir "output/bsmlimits/TopPhilicPseudoScalarSinglet"
fi
if [ ! -d "output/bsmlimits/TopPhilicVectorSinglet" ]
then
    mkdir "output/bsmlimits/TopPhilicVectorSinglet"
fi
if [ ! -d "output/bsmlimits/TopPhilicScalarOctet" ]
then
    mkdir "output/bsmlimits/TopPhilicScalarOctet"
fi
if [ ! -d "output/bsmlimits/TopPhilicPseudoScalarOctet" ]
then
    mkdir "output/bsmlimits/TopPhilicPseudoScalarOctet"
fi
if [ ! -d "output/bsmlimits/TopPhilicVectorOctet" ]
then
    mkdir "output/bsmlimits/TopPhilicVectorOctet"
fi

RunDatacardCreation ScalarSinglet 0p4 2018 400
RunDatacardCreation ScalarSinglet 0p6 2018 600
RunDatacardCreation ScalarSinglet 0p8 2018 800
RunDatacardCreation ScalarSinglet 1p0 2018 1000
RunDatacardCreation ScalarSinglet 1p5 2018 1500

# create datacards for each BSM model
# Output should be specified in a dcf template?