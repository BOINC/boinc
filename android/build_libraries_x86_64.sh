#!/bin/sh
set -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildApp#
#

# Script to compile various BOINC libraries for Android to be used
# by science applications

STDOUT_TARGET="${STDOUT_TARGET:-/dev/stdout}"
COMPILEBOINC="yes"
CONFIGURE="yes"
MAKECLEAN="yes"
VERBOSE="${VERBOSE:-no}"

export BOINC=".." #BOINC source code

export NDK_ROOT=${NDK_ROOT:-$HOME/Android/Ndk}
export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROIDTC="${ANDROID_TC_X86_64:-$ANDROID_TC/x86_64}"
export TOOLCHAINROOT="$NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/"
export TCBINARIES="$TOOLCHAINROOT/bin"
export TCINCLUDES="$ANDROIDTC/x86_64-linux-android"
export TCSYSROOT="$TOOLCHAINROOT/sysroot"

export PATH="$TCBINARIES:$TCINCLUDES/bin:$PATH"
export CC=x86_64-linux-android21-clang
export CXX=x86_64-linux-android21-clang++
export LD=x86_64-linux-android-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -DANDROID_64 -DDECLARE_TIMEZONE -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE -D__ANDROID_API__=21"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -DANDROID_64 -Wall -I$TCINCLUDES/include -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE -D__ANDROID_API__=21"
export LDFLAGS="-L$TCSYSROOT/usr/lib64 -L$TCINCLUDES/lib64 -llog -fPIE -pie -latomic -static-libstdc++"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"
export PKG_CONFIG_SYSROOT_DIR="$TCSYSROOT"

if [ -n "$COMPILEBOINC" ]; then

    cd "$BOINC"
    echo "===== building BOINC Libraries for x86-64 from $PWD ====="
    if [ -n "$MAKECLEAN" ] && [ -f "Makefile" ]; then
        if [ "$VERBOSE" = "no" ]; then
            make distclean 1>$STDOUT_TARGET 2>&1
        else
            make distclean SHELL="/bin/bash -x"
        fi
    fi
    if [ -n "$CONFIGURE" ]; then
        ./_autosetup
        ./configure --host=x86_64-linux --with-boinc-platform="x86_64-android-linux-gnu" --with-boinc-alt-platform="x86-android-linux-gnu" --prefix="$TCINCLUDES" --libdir="$TCINCLUDES/lib" --with-ssl="$TCINCLUDES" --disable-server --disable-manager --disable-client --disable-shared --enable-static --enable-boinczip
    fi
    if [ "$VERBOSE" = "no" ]; then
        make --silent
        make stage --silent
        make install --silent
    else
        make SHELL="/bin/bash -x"
        make stage SHELL="/bin/bash -x"    
        make install SHELL="/bin/bash -x"    
    fi
    echo "===== building BOINC Libraries for x86-64 done ====="

fi
