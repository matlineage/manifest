Features / Changes (from LineageOS):
* spectrum visualizer on navigation bar and lock screen
* colored statusbar notification icons
* transient task mode aka tadimode
* clear all button on recents screen (instead of screenshot)
* disable "Managed if unused"
* remove "app continuity" nonsense in application info
* don't harass the user with "discover trust"
* ask less unnecessary questions in setup wizard
* disable "recommended volume level" warning/nagging
* changed defaults to enable developer options, usb debugging and rooted debugging and disable ADB authorization expiry

Future plans / Goals:
* faster unlock (on phones i own, apparently it's quite device specific)
* nice font (or perhaps just font selection thing)
* extra timezones on lock screen clock, maybe weather too, some way
* advanced reboot on lockscreen option

* settings page with options for:
	* spectrum on/off (separate options for lockscreen and for statusbar)
	* spectrum width (less than 20kHz to look nicer)
	* color statusbar icons
	* font
	* timezones on lockscreen?

* default on for advanced restart
* perhaps default off widgets on lock screen
* less transparant spectrum, or configurable transparency level

* default on battery percentage visible? ("inside the icon")
* the harassment about summarizing notifications should go away
* turn notification organizer off by default
* default off battery manager
* default off "enhanced notifications"
* default off notification cooldown
* default to "full list" notifications on lock screen
* remove "explore hub mode" nag screen

What files here are:
* README.MD - This file.
* preparechroot.sh - Script to create user inside chroot.
* becomebuntu.sh - Script to use a chroot but with current home dir bind-mounted.
* mchroot.c - Custom chroot utility, used by becomebuntu.sh, preparechroot.sh.
* build.sh - Script to build for a specified device, with separate out directory.
* build-filter-vendors.sh - Used by build.sh to avoid issues by fighting the build system.
* prompt_prefix.txt - What i feed LLMs as a preamble in an attempt to make them behave a bit.
* snapshot.xml - Has output of `repo manifest -r -o snapshot.xml` to perhaps help diagnose build issues in future.

Order of doing things to get a build:
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
* Actually build: `./build.sh <target_device>`
	* The point of the script is to have separate output directories so incremental builds can be done for multiple devices without rebuilding too much.
	* Alternatively just follow the instructions for your device.

Miscellaneous notes:
* To push anything on a fresh fetch, have to first get the repository ready with `repo start lineage-24.0 frameworks/base` (or other dir for other repo).
* There are TWO ways of navigation bar existing "classic navigation bar" and "taskbar", only "taskbar" is relevant in current lineage it seems.
	* this is set in AOSP default release configuration -> enable_taskbar_on_phones -> enableTaskbarOnPhones() (platform/build/release repository)
	* cp2a (android 17) inherits it from cp1a <- bp4a <- bp3a <- bp2a <- bp1a <- enable_taskbar_on_phones
	* so the spectrum analyzer thing only targets that via taskbardelegate.java
