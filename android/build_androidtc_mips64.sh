#/bin/sh

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to setup Android toolchain

export NDKROOT="/home/boincadm/NVPACK/android-ndk-r10e"
export ANDROIDTC="$HOME/androidmips64-tc"

if [ ! -d $ANDROIDTC/mips64el-linux-android ]; then
    $NDKROOT/build/tools/make-standalone-toolchain.sh --platform=android-21 --arch=mips64 --install-dir=$ANDROIDTC
fi
