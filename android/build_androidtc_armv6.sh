#!/bin/sh
set -e

#
# See: https://github.com/BOINC/boinc/wiki/AndroidBuildClient#
#

# Script to setup Android toolchain

export NDK_ARMV6_ROOT="${NDK_ARMV6_ROOT:-$HOME/NVPACK/android-ndk-r11}"
export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROID_TC_ARMV6="${ANDROID_TC_ARMV6:-$ANDROID_TC/armv6}"

if [ ! -d "$ANDROID_TC_ARMV6/arm-linux-androideabi" ]; then
    "$NDK_ARMV6_ROOT/build/tools/make-standalone-toolchain.sh" --verbose --toolchain=arm-linux-androideabi-clang --platform=android-16 --arch=arm --stl=libc++ --install-dir="$ANDROID_TC_ARMV6" "$@"
fi
