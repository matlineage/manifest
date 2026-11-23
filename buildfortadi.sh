#!/bin/bash
set -e
ln -sfnT out_tadi out
. build/envsetup.sh
lunch lineage_TP1803
source build-filter-vendors.sh
m bacon
