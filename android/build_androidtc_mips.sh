#/bin/sh

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to setup Android toolchain

export NDK_ROOT="${NDK_ROOT:-$HOME/NVPACK/android-ndk-r10e}"
export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROID_TC_MIPS="${ANDROID_TC_MIPS:-$ANDROID_TC/mips}"

if [ ! -d $ANDROID_TC_MIPS/mipsel-linux-android ]; then
    $NDK_ROOT/build/tools/make-standalone-toolchain.sh --verbose --platform=android-16 --arch=mips --install-dir=$ANDROID_TC_MIPS
    if [ $? -ne 0 ]; then exit 1; fi
fi
