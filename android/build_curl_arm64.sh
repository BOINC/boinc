#!/bin/sh
set -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile Libcurl for Android

COMPILECURL="${COMPILECURL:-yes}"
STDOUT_TARGET="${STDOUT_TARGET:-/dev/stdout}"
CONFIGURE="yes"
MAKECLEAN="yes"
VERBOSE="${VERBOSE:-no}"

CURL="${CURL_SRC:-$HOME/src/curl-7.61.0}" #CURL sources, required by BOINC

export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROIDTC="${ANDROID_TC_ARM:-$ANDROID_TC}"
export TCBINARIES="$ANDROIDTC/toolchains/llvm/prebuilt/linux-x86_64/bin"
export TCINCLUDES="$ANDROIDTC/prebuilt/linux-x86_64/"
export TCSYSROOT="$ANDROIDTC/toolchains/llvm/prebuilt/linux-x86_64/sysroot"
export STDCPPTC=""

export PATH="$TCBINARIES:$TCINCLUDES/bin:$PATH"
export CC=aarch64-linux-android21-clang
export CXX=aarch64-linux-android21-clang++
export LD=aarch64-linux-android-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE -D__ANDROID_API__=21"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE -D__ANDROID_API__=21"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog -fPIE -pie -latomic -static-libstdc++"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"

# Prepare android toolchain and environment

if [ "$COMPILECURL" = "yes" ]; then
    cd "$CURL"
    echo "===== building curl for arm64 from $PWD ====="    
    if [ -n "$MAKECLEAN" ] && $(grep -q "^distclean:" "${CURL}/Makefile"); then
        if [ "$VERBOSE" = "no" ]; then
            make distclean 1>$STDOUT_TARGET 2>&1
        else
            make distclean SHELL="/bin/bash -x"
        fi
    fi
    if [ -n "$CONFIGURE" ]; then
        ./configure --host=aarch64-linux --prefix="$TCINCLUDES" --libdir="$TCINCLUDES/lib" --disable-shared --enable-static --with-random=/dev/urandom 1>$STDOUT_TARGET
    fi
    if [ "$VERBOSE" = "no" ]; then
        make --silent 1>$STDOUT_TARGET
        make install --silent 1>$STDOUT_TARGET
    else
        make SHELL="/bin/bash -x"
        make install SHELL="/bin/bash -x"
    fi
    echo "===== curl for arm64 build done ====="
fi
