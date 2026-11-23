#!/bin/bash
set -e
mkdir -p out_tadi
ln -sfnT out_tadi out
. build/envsetup.sh
lunch lineage_TP1803-cp2a-userdebug
source build-filter-vendors.sh
m bacon
