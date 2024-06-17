#!/bin/bash

python3 buildDatacard.py -vf settingfiles/Variables/base.json -sf settingfiles/Uncertainties/2018.json -pf settingfiles/Process/SM.json -cf settingfiles/Channel/all_channels.json -y 2018 -dcf settingfiles/Datacards/2018_SM.json
python3 buildDatacard.py -vf settingfiles/Variables/base.json -sf settingfiles/Uncertainties/2017.json -pf settingfiles/Process/SM.json -cf settingfiles/Channel/all_channels.json -y 2017 -dcf settingfiles/Datacards/2017_SM.json
python3 buildDatacard.py -vf settingfiles/Variables/base.json -sf settingfiles/Uncertainties/2016.json -pf settingfiles/Process/SM.json -cf settingfiles/Channel/all_channels.json -y 2016 -dcf settingfiles/Datacards/2016_SM.json

