#!/bin/sh
set -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to setup Android toolchain

export NDK_ROOT="${NDK_ROOT:-$HOME/NVPACK/android-ndk-r10e}"
export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROID_TC_X86="${ANDROID_TC_X86:-$ANDROID_TC/x86}"
IS_MAKE_STANDALONE=false

if [ ! -d "${ANDROID_TC_X86}/i686-linux-android" ]; then
    IS_MAKE_STANDALONE=true
fi
if [ ! -z ${NDK_STANDALONTOOL_FLAGFILE} ]; then
    if [  ! -e "${NDK_STANDALONTOOL_FLAGFILE}" ]; then
        IS_MAKE_STANDALONE=true
    else
        IS_MAKE_STANDALONE=false
    fi
fi

if $IS_MAKE_STANDALONE ; then
    rm -rf $ANDROID_TC_X86
    "${NDK_ROOT}/build/tools/make-standalone-toolchain.sh" --verbose --platform=android-16 --arch=x86 --stl=libc++ --install-dir="${ANDROID_TC_X86}" "$@"
fi

if [ ! -z ${NDK_STANDALONTOOL_FLAGFILE} ]; then
    touch ${NDK_STANDALONTOOL_FLAGFILE}
fi
