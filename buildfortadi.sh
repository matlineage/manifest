#!/bin/bash
set -e

# Make sure to be in the right dir.
root=$(realpath "$0") || exit 1
root=$(dirname "$root") || exit 1
root=$(realpath "$root") || exit 1
cd $root"/lineage"

mkdir -p out_tadi
ln -sfnT out_tadi out
. build/envsetup.sh
lunch lineage_TP1803-cp2a-userdebug
source build-filter-vendors.sh
m bacon
