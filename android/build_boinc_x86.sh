#!/bin/sh
set -e

#
# See: https://github.com/BOINC/boinc/wiki/AndroidBuildClient#
#

# Script to compile BOINC for Android

STDOUT_TARGET="${STDOUT_TARGET:-/dev/stdout}"
COMPILEBOINC="yes"
CONFIGURE="yes"
MAKECLEAN="yes"
VERBOSE="${VERBOSE:-no}"
NPROC_USER="${NPROC_USER:-1}"

export BOINC=".." #BOINC source code

export NDK_ROOT=${NDK_ROOT:-$HOME/Android/Ndk}
export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROIDTC="${ANDROID_TC_X86:-$ANDROID_TC/x86}"
export TOOLCHAINROOT="$NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64"
export TCBINARIES="$TOOLCHAINROOT/bin"
export TCINCLUDES="$ANDROIDTC/i686-linux-android"
export TCSYSROOT="$TOOLCHAINROOT/sysroot"
export VCPKG_DIR=$VCPKG_ROOT/installed/x86-android

CONFIG_FLAGS=""
CONFIG_LDFLAGS=""
CONFIG_CXXFLAGS=""

CONFIG_LDFLAGS="-L$VCPKG_DIR/lib"
CONFIG_CXXFLAGS="-I$VCPKG_DIR/include"
CONFIG_FLAGS="--with-ssl=$VCPKG_DIR --with-libcurl=$VCPKG_DIR"
export _libcurl_pc="$VCPKG_DIR/lib/pkgconfig/libcurl.pc"
export PKG_CONFIG_PATH=$VCPKG_DIR/lib/pkgconfig/

export PATH="$TCBINARIES:$TCINCLUDES/bin:$PATH"
export CC=i686-linux-android16-clang
export CXX=i686-linux-android16-clang++
export LD=i686-linux-android-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -DDECLARE_TIMEZONE -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE -D__ANDROID_API__=16"
export CXXFLAGS="$CONFIG_CXXFLAGS --sysroot=$TCSYSROOT -std=c++11 -DANDROID -Wall -I$TCINCLUDES/include -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE -D__ANDROID_API__=16"
export LDFLAGS="$CONFIG_LDFLAGS -llog -fPIE -pie -latomic -static-libstdc++"
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
    echo "===== building BOINC for x86 from $PWD ====="
    if [ -n "$MAKECLEAN" ] && [ -f "Makefile" ]; then
        if [ "$VERBOSE" = "no" ]; then
            make distclean 1>$STDOUT_TARGET 2>&1
        else
            make distclean SHELL="/bin/bash -x"
        fi
    fi
    if [ -n "$CONFIGURE" ]; then
        ./_autosetup
        ./configure --host=i686-linux --with-boinc-platform="x86-android-linux-gnu" $CONFIG_FLAGS --disable-server --disable-manager --disable-shared --enable-static --disable-largefile
    fi
    echo MAKE_FLAGS=$MAKE_FLAGS
    make $MAKE_FLAGS
    make stage $MAKE_FLAGS

    echo "Stripping Binaries"
    cd stage/usr/local/bin
    llvm-strip *
    cd ../../../../

    echo "Copy Assets"
    cd android
    mkdir -p "BOINC/app/src/main/assets"
    cp "$BOINC/stage/usr/local/bin/boinc" "BOINC/app/src/main/assets/x86/boinc"
    cp "$BOINC/win_build/installerv2/redist/all_projects_list.xml" "BOINC/app/src/main/assets/all_projects_list.xml"
    cp "$BOINC/curl/ca-bundle.crt" "BOINC/app/src/main/assets/ca-bundle.crt"

    echo "\e[1;32m===== BOINC for x86 build done =====\e[0m"

fi
