#!/bin/sh

# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2025 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
#

set -e

#
# See: https://github.com/BOINC/boinc/wiki/AndroidBuildApp
#

# Script to compile various BOINC libraries for Android to be used
# by science applications

STDOUT_TARGET="${STDOUT_TARGET:-/dev/stdout}"
COMPILEBOINC="yes"
CONFIGURE="yes"
MAKECLEAN="yes"
VERBOSE="${VERBOSE:-no}"
NPROC_USER="${NPROC_USER:-1}"

export BOINC=".." #BOINC source code

export NDK_ROOT=${NDK_ROOT:-$HOME/Android/Ndk}
export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROIDTC="${ANDROID_TC_ARM:-$ANDROID_TC/arm}"
export TOOLCHAINROOT="$NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64"
export TCBINARIES="$TOOLCHAINROOT/bin"
export TCINCLUDES="$ANDROIDTC/arm-linux-androideabi"
export TCSYSROOT="$TOOLCHAINROOT/sysroot"
export VCPKG_DIR=$VCPKG_ROOT/installed/arm-android

CONFIG_FLAGS=""
CONFIG_LDFLAGS=""

CONFIG_LDFLAGS="-L$VCPKG_DIR/lib"
CONFIG_FLAGS="--with-ssl=$VCPKG_DIR --with-libcurl=$VCPKG_DIR"
export _libcurl_pc="$VCPKG_DIR/lib/pkgconfig/libcurl.pc"
export PKG_CONFIG_PATH=$VCPKG_DIR/lib/pkgconfig/

export PATH="$TCBINARIES:$TCINCLUDES/bin:$PATH"
export CC=armv7a-linux-androideabi16-clang
export CXX=armv7a-linux-androideabi16-clang++
export LD=arm-linux-androideabi-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -DDECLARE_TIMEZONE -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -D__ANDROID_API__=16"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -I$TCINCLUDES/include -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -D__ANDROID_API__=16"
export LDFLAGS="$CONFIG_LDFLAGS -L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog -fPIE -pie -latomic -static-libstdc++ -march=armv7-a -Wl,--fix-cortex-a8"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"

MAKE_FLAGS=""

if [ $VERBOSE = "no" ]; then
    MAKE_FLAGS="$MAKE_FLAGS --silent"
else
    MAKE_FLAGS="$MAKE_FLAGS SHELL=\"/bin/bash -x\""
fi

if [ $CI = "yes" ]; then
    MAKE_FLAGS="$MAKE_FLAGS -j $(nproc --all)"
else
    MAKE_FLAGS="$MAKE_FLAGS -j $NPROC_USER"
fi

if [ -n "$COMPILEBOINC" ]; then

    cd "$BOINC"
    echo "===== building BOINC Libraries for arm from $PWD ====="
    if [ -n "$MAKECLEAN" ] && [ -f "Makefile" ]; then
        if [ "$VERBOSE" = "no" ]; then
            make distclean 1>$STDOUT_TARGET 2>&1
        else
            make distclean SHELL="/bin/bash -x"
        fi
    fi
    if [ -n "$CONFIGURE" ]; then
        ./_autosetup
        ./configure --host=arm-linux --with-boinc-platform="arm-android-linux-gnu" --prefix="$TCINCLUDES" $CONFIG_FLAGS --enable-boinczip --disable-server --disable-manager --disable-client --disable-shared --enable-static --disable-largefile
    fi
    echo MAKE_FLAGS=$MAKE_FLAGS
    make $MAKE_FLAGS
    make stage $MAKE_FLAGS
    make install $MAKE_FLAGS
    echo "\e[1;32m===== building BOINC Libraries for arm done =====\e[0m"

fi
