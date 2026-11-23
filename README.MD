What files here are:
* README.MD - this file
* preparechroot.sh - script to create user inside chroot
* becomebuntu.sh - script to use a chroot but with current home dir bind-mounted
* mchroot.c - custom chroot utility, used by becomebuntu.sh, preparechroot.sh
* buildfortadi.sh, buildforsony.sh - scripts to build for my specific phones
* build-filter-vendors.sh - used by buildfor*.sh to avoid issues by fighting the build system
* prompt_prefix.txt - what i feed LLMs as a preamble in an attempt to make them behave a bit
* snapshot.xml has output of `repo manifest -r -o snapshot.xml` to perhaps help diagnose build issues in future

Order of doing things:
* Check out this repo and cd to it.
* Create ubuntu chroot in buildenv dir (optional I suppose):
```
# we need debootstrap, make, gcc
sudo debootstrap stonking buildenv
make mchroot
./preparechroot.sh
```
* Install dependencies:
```
sudo ./becomebuntu.sh
apt install -y software-properties-common
add-apt-repository universe
apt update
apt install -y bc bison build-essential ccache curl erofs-utils flex g++-multilib gcc-multilib git git-lfs gnupg gperf imagemagick kmod protobuf-compiler python3-protobuf lib32readline-dev lib32z1-dev libdw-dev libelf-dev libgnutls28-dev lz4 libsdl1.2-dev libssl-dev libxml2-16 libxml2-utils lzop pngcrush rsync schedtool squashfs-tools xsltproc xxd zip zlib1g-dev
apt install -y adb fastboot # optional depending on your situation
curl https://storage.googleapis.com/git-repo-downloads/repo > /usr/bin/repo
chmod a+x /usr/bin/repo
exit # run ./becomebuntu.sh as normal user later
```
* Check out the sources from lineage and from this project also.
```
./becomebuntu.sh
mkdir -p lineage/.repo
cd lineage
ln -s ../../manifests .repo/local_manifests
repo init -u https://github.com/LineageOS/android.git -b lineage-24.0 --git-lfs --no-clone-bundle
repo sync -j8 # Limit the amount of threads because the servers get cranky otherwise.
```
* Optionally set up ccache for faster building:
```
export USE_CCACHE=1
export CCACHE_EXEC=/usr/bin/ccache
ccache -M 50G
```
* Propriatary files have to be taken some way, how that's done depends on the device port.
    * For Sony pdx246: `./device/sony/pdx246/setup.sh /path/to/stock/dump`
    * For TP1803: `cd device/nubia/TP1803 && ./extract-files.py` with the device connected (uses adb).
* Actually build:


Miscellaneous notes:
* To push anything on a fresh fetch, have to first get the repository ready with `repo start lineage-24.0 frameworks/base` (or other dir for other repo).
