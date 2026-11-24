#!/bin/sh
set -e
root=$(realpath "$0") || exit 1
root=$(dirname "$root") || exit 1
root=$(realpath "$root") || exit 1
targetpwd=$PWD
cd "$root"
user=$(whoami)
uid=$(id -u)
sudo sh -c "touch buildenv/etc/resolv.conf && chown $user buildenv/etc/resolv.conf && ./mchroot -r buildenv /bin/sh -c 'set -e; useradd -u $uid -g 0 -s /bin/bash $user'"
