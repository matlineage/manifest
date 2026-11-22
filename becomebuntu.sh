#!/bin/sh
set -e
root=$(realpath "$0") || exit 1
root=$(dirname "$root") || exit 1
root=$(realpath "$root") || exit 1
targetpwd=$PWD
cd "$root"
# buildenv dir should contain a chroot with a user of the same UID and username as current
user=$(whoami)
sudo sh -c "cp /etc/resolv.conf buildenv/etc/resolv.conf; ./mchroot -P /home -P /tmp buildenv /bin/sh -c 'cd $targetpwd; exec runuser $user'"
