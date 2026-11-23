#!/bin/bash
set -e

# Make sure to be in the right dir.
root=$(realpath "$0") || exit 1
root=$(dirname "$root") || exit 1
root=$(realpath "$root") || exit 1
cd $root"/lineage"

mkdir -p out_sony
ln -sfnT out_sony out
. build/envsetup.sh
lunch lineage_pdx246-cp2a-userdebug
source build-filter-vendors.sh
m bacon
