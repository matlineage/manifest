#!/bin/bash
set -e

# Error if no argument.
if [[ $# -eq 0 ]] ; then
	echo "usage: $0 <target_device>"
	exit 1
fi

# Make sure to be in the right dir.
root=$(realpath "$0") || exit 1
root=$(dirname "$root") || exit 1
root=$(realpath "$root") || exit 1
cd "$root/lineage"

mkdir -p "out_$1"
ln -sfnT "out_$1" out # Yes, I know about OUT_DIR but it caused issues.
. build/envsetup.sh
lunch "lineage_$1-cp2a-userdebug"
source "$root/build-filter-vendors.sh"
m bacon
