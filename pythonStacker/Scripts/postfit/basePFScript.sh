#!/bin/bash

source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /user/nivanden/CMSSW_11_3_4/src
eval `scramv1 runtime -sh`
export X509_USER_PROXY=/user/$USER/x509up_u$(id -u $USER)
cd /user/nivanden/Stacker_v2/postfit/variables/PLOT

ulimit -S -s unlimited

if [ ! -f "PLOT_shape.root" ]; then
    combineCards.py ch1_y16_y16=DC_2016_flav.txt ch1_y17_y17=DC_2017_flav.txt ch1_y18_y18=DC_2018_flav.txt > PLOT.txt
    text2workspace.py -P HiggsAnalysis.CombinedLimit.PhysicsModel:multiSignalModel  --PO 'map=.*/TTTT:r_TTTT[1,-5,5]' --PO 'map=.*/TTW:r_TTW[1,-5,5]' --PO 'map=.*/TTZ:r_TTZ[1,-5,5]' -o PLOT.root PLOT.txt
    PostFitShapesFromWorkspace -d PLOT.txt -w PLOT.root --output /user/nivanden/Stacker_v2/postfit/variables/PLOT/PLOT_shape.root --postfit -f /user/nivanden/Stacker_v2/postfit/inputs/fitDiagnosticsobs_all_srFit.txt.root:fit_s --total-shapes --sampling
fi
cd /user/nivanden/Stacker_v2/postfit/

python drawPF.py \
--file="/user/nivanden/Stacker_v2/postfit/variables/PLOT/PLOT_shape.root" \
--mode="postfit" \
--ratio \
--split \
--extra_pad=0.4 \
--ratio_range 0.1,1.9 \
--outname "variables/PLOT/postfit_PLOT"

python drawPF.py \
--file="/user/nivanden/Stacker_v2/postfit/variables/PLOT/PLOT_shape.root" \
--mode="prefit" \
--ratio \
--split \
--extra_pad=0.4 \
--ratio_range 0.1,1.9 \
--outname "variables/PLOT/prefit_PLOT"
