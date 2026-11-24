#!/bin/sh
set -e
root=$(realpath "$0") || exit 1
root=$(dirname "$root") || exit 1
root=$(realpath "$root") || exit 1
targetpwd=$PWD
cd "$root"
# buildenv dir should contain a chroot with a user of the same UID as current
user=$(whoami)
./mchroot -P /home -P /tmp -p /etc/resolv.conf -r buildenv -n -c "$targetpwd" /bin/bash
