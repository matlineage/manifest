#!/bin/bash
set -e
ln -sfnT out_sony out
. build/envsetup.sh
lunch lineage_pdx246-cp2a-userdebug
source build-filter-vendors.sh
m bacon
