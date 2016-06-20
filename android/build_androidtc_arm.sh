#/bin/sh

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to setup Android toolchain

export NDKROOT="/home/boincadm/NVPACK/android-ndk-r9d"
export ANDROIDTC="$HOME/androidarm-tc"

if [ ! -d $ANDROIDTC/arm-linux-androideabi ]; then
    $NDKROOT/build/tools/make-standalone-toolchain.sh --platform=android-9 --arch=arm --install-dir=$ANDROIDTC
fi
