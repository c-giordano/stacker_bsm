#!/bin/bash

python drawPF.py \
--file="/user/nivanden/Stacker_v2/postfit/variables/njets_sr_mumu/njets_pf_out.root" \
--mode="postfit" \
--ratio \
--split \
--extra_pad=0.20 \
--ratio_range 0.1,2.9 \
--outname "test_all"


python drawPF.py \
--file="/user/nivanden/Stacker_v2/postfit/variables/njets_sr_mumu/njets_pf_out_tttt.root" \
--mode="postfit" \
--ratio \
--split \
--extra_pad=0.20 \
--ratio_range 0.1,3.9 \
--outname "test_tttt"


python drawPF.py \
--file="/user/nivanden/Stacker_v2/postfit/variables/njets_sr_mumu/njets_pf_out_ttx.root" \
--mode="postfit" \
--ratio \
--split \
--extra_pad=0.20 \
--ratio_range 0.1,3.4 \
--outname "test_ttx"