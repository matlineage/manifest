To build:
* check out this repo and cd to it
* run:
```
mkdir -p lineage/.repo
cd lineage
ln -s ../../manifests .repo/local_manifests
repo init -u https://github.com/LineageOS/android.git -b lineage-24.0 --git-lfs --no-clone-bundle
repo sync -j8
```
*