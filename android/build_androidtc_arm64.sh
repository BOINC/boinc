#!/bin/sh
set -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to setup Android toolchain

export NDK_ROOT="${NDK_ROOT:-$HOME/NVPACK/android-ndk-r10e}"
export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROID_TC_ARM64="${ANDROID_TC_ARM64:-$ANDROID_TC/arm64}"

if [ ! -d "$ANDROID_TC_ARM64/aarch64-linux-android" ]; then
    "$NDK_ROOT/build/tools/make-standalone-toolchain.sh" --verbose --platform=android-21 --arch=arm64 --stl=libc++ --install-dir="$ANDROID_TC_ARM64" "$@"
fi
