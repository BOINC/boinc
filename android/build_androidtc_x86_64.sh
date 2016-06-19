#/bin/sh

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to setup Android toolchain

export NDKROOT="/home/boincadm/NVPACK/android-ndk-r10e"
export ANDROIDTC="$HOME/androidx86_64-tc"

if [ ! -d $ANDROIDTC/x86_64-linux-android ]; then
    $NDKROOT/build/tools/make-standalone-toolchain.sh --platform=android-21 --arch=x86_64 --install-dir=$ANDROIDTC
fi
