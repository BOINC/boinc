#!/bin/sh
set -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to setup Android toolchain

export NDK_ROOT="${NDK_ROOT:-$HOME/NVPACK/android-ndk-r10e}"
export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROID_TC_ARM="${ANDROID_TC_ARM:-$ANDROID_TC/arm}"

if [ ! -d "$ANDROID_TC_ARM/arm-linux-androideabi" ]; then
    "$NDK_ROOT/build/tools/make-standalone-toolchain.sh" --verbose --platform=android-19 --arch=arm --stl=libc++ --install-dir="$ANDROID_TC_ARM" "$@"
fi
