#/bin/sh -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to setup Android toolchain

export NDK_ROOT="${NDK_ROOT:-$HOME/NVPACK/android-ndk-r10e}"
export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROID_TC_MIPS="${ANDROID_TC_MIPS:-$ANDROID_TC/mips}"

<<<<<<< HEAD
if [ ! -d "$ANDROID_TC_MIPS/mipsel-linux-android" ]; then
    "$NDK_ROOT/build/tools/make-standalone-toolchain.sh" --verbose --platform=android-14 --arch=mips --install-dir="$ANDROID_TC_MIPS" "$@"
=======
if [ ! -d $ANDROID_TC_MIPS/mipsel-linux-android ]; then
    $NDK_ROOT/build/tools/make-standalone-toolchain.sh --verbose --platform=android-14 --arch=mips --install-dir=$ANDROID_TC_MIPS
>>>>>>> android_ci
fi
