#/bin/sh

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to setup Android toolchain

export NDK_ROOT="${NDK_ROOT:-$HOME/NVPACK/android-ndk-r10e}"
export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROID_TC_MIPS64="${ANDROID_TC_MIPS64:-$ANDROID_TC/mips64}"

if [ ! -d $ANDROID_TC_MIPS64/mips64el-linux-android ]; then
    $NDK_ROOT/build/tools/make-standalone-toolchain.sh --platform=android-21 --arch=mips64 --install-dir=$ANDROID_TC_MIPS64
fi
