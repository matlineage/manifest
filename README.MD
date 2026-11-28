Features / Changes (from LineageOS):
* Spectrum visualizer on navigation bar and lock screen. (frameworks/base)
* Colored statusbar notification icons (stolen from crDroid). (frameworks/base)
* Transient task mode aka tadimode (remove apps from recents when pressing back to exit them). (frameworks/base)
	* Stolen from LMO (https://gerrit.libremobileos.com/c/LMODroid/platform_frameworks_base/+/23084)
* Clear all button on recents screen (instead of screenshot). (packages/apps/Launcher3)
* Disable "Managed if unused" from application info. (frameworks/base, packages/apps/Settings, packages/modules/Permission)
	* It is in 2 commits, the second part also disables it being enabled at run-time in case of dirty-flashing or whatever.
* Remove "App Continuity" nonsense in application info. (packages/apps/Settings)
* Don't harass the user with "Discover trust" (notification on first boot). (lineage-sdk, packages/apps/LineageParts)
* Ask less unnecessary questions in setup wizard. (packages/apps/SetupWizard)
* Disable "Recommended volume level" warning/nagging. (frameworks/base)
* Changed defaults to enable developer options, usb debugging and rooted debugging and disable ADB authorization expiry.
* Disabled by default: "Enhanced notifications", "Notification cooldown"
* Default to "Full list" notifications on lock screen.
* Set show battery percentage inside icon by default.
* Turned off "Battery Manager" by default.
* Turned off "Notification organiser" by default.
* Hid "Battery Manager", "Notification summaries" and "Notification organiser" from settings (they're useless/defunct anyway).
* Disabled "Notification summaries" and "Notification organiser" onboarding nagging in notification tray.
* Disabled "Explore hub mode" nag screen (when going to widgets pane on lock screen).
* Turned off "Widgets on lock screen" by default.
* Turned on "Advanced restart" by default.
* Advanced restart on lockscreen.
* Disabled the first-boot home hotseat discovery bounce in Launcher3 (icons bouncing on first boot).
(TEST) * Default to small clock on lock screen even when no notifications are shown (display-> lock screen -> dynamic clock).
(TEST) * Full granularity statusbar icons for wifi/signal level.
(TEST) * LMODroid's default keyboard which has swipe.
* Mat extras settings page:
	* Spectrum on/off (statusbar + lockscreen).
	* Spectrum width (frequency).
	* Lock screen spectrum height.
	* Use mic input for spectrum.
	* Color statusbar icons on/off. (also in normal settings -> system -> status bar)
	* Font selection (also in settings -> wallpaper and style, then swipe to home screen).

Future plans / Goals:
* faster unlock (on phones i own, apparently it's quite device specific)
* extra timezones on lock screen clock, maybe weather too, some way
* settings page with options for:
	* timezones on lockscreen?
* show seconds on clocks by default?
* notification count with statusbar icons
* GSI build (allegedly just gsi_arm64 target)

What files here are:
* README.MD - This file.
* preparechroot.sh - Script to create user inside chroot (run as the user you intend to use for building, will sudo).
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
* Optionally set up ccache for faster building (have to run each session or add to profile perhaps):
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
	* so the spectrum analyzer thing only targets that via TaskbarDelegate.java
* For debugging perhaps useful: `../build_systemonly.sh TP1803 && adb reboot fastboot && fastboot flash system out/target/product/TP1803/system.img && fastboot flash system_ext out/target/product/TP1803/system_ext.img && fastboot flash product out/target/product/TP1803/product.img && fastboot reboot`

Credit:
	Many thanks to LineageOS, LMODroid, crDroid and InfinityX projects, for the code and inspiration used.
