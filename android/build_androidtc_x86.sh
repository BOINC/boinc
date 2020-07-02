#!/bin/sh
set -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to setup Android toolchain

export NDK_ROOT="${NDK_ROOT:-$HOME/NVPACK/android-ndk-r10e}"
export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROID_TC_X86="${ANDROID_TC_X86:-$ANDROID_TC/x86}"
CREATE_TOOLCHAIN="no"

if [ ! -d "${ANDROID_TC_X86}/i686-linux-android" ]; then
    CREATE_TOOLCHAIN="yes"
fi

if [ -n $NDK_TOOLCHAIN_FLAGFILE  ]; then
    if [ ! -e "${NDK_TOOLCHAIN_FLAGFILE}" ]; then
        CREATE_TOOLCHAIN="yes"
    else
        CREATE_TOOLCHAIN="no"
    fi
fi

if [ $CREATE_TOOLCHAIN = "yes" ]; then
    "${NDK_ROOT}/build/tools/make-standalone-toolchain.sh" --verbose --platform=android-16 --arch=x86 --stl=libc++ --install-dir="${ANDROID_TC_X86}" "$@"
fi

if [ -n $NDK_TOOLCHAIN_FLAGFILE  ]; then
    touch $NDK_TOOLCHAIN_FLAGFILE
fi
