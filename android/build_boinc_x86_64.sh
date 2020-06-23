#!/bin/sh
set -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile BOINC for Android

STDOUT_TARGET="${STDOUT_TARGET:-/dev/stdout}"
COMPILEBOINC="yes"
CONFIGURE="yes"
MAKECLEAN="yes"
VERBOSE="${VERBOSE:-no}"

export BOINC=".." #BOINC source code

export ANDROID_NDK="${ANDROID_NDK:-$HOME/android-ndk}"
export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROIDTC="${ANDROID_TC_X86_64:-$ANDROID_TC/x86_64}"
export TCBINARIES="$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin"
export TCINCLUDES="$ANDROIDTC/x86_64-linux-android"
export TCSYSROOT="$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot"
export STDCPPTC=""

export PATH="$TCBINARIES:$TCINCLUDES/bin:$PATH"
export CC=x86_64-linux-android21-clang
export CXX=x86_64-linux-android21-clang++
export LD=x86_64-linux-android-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -DANDROID_64 -DDECLARE_TIMEZONE -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE -D__ANDROID_API__=21"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -DANDROID_64 -Wall -I$TCINCLUDES/include -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE -D__ANDROID_API__=21"
export LDFLAGS="-L$TCSYSROOT/usr/lib64 -L$TCINCLUDES/lib64 -llog -fPIE -pie -latomic -static-libstdc++"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"
export PKG_CONFIG_SYSROOT_DIR="$TCSYSROOT"

# Prepare android toolchain and environment

if [ -n "$COMPILEBOINC" ]; then
    cd "$BOINC"
    echo "===== building BOINC for x86-64 from $PWD ====="    
    if [ -n "$MAKECLEAN" ] && [ -f "Makefile" ]; then
        if [ "$VERBOSE" = "no" ]; then
            make distclean 1>$STDOUT_TARGET 2>&1
        else
            make distclean SHELL="/bin/bash -x"
        fi
    fi
    if [ -n "$CONFIGURE" ]; then
        ./_autosetup
        ./configure --host=x86_64-linux --with-boinc-platform="x86_64-android-linux-gnu" --with-boinc-alt-platform="x86-android-linux-gnu" --with-ssl="$TCINCLUDES" --disable-server --disable-manager --disable-shared --enable-static
        sed -e "s%^CLIENTLIBS *= *.*$%CLIENTLIBS = -lm $STDCPPTC%g" client/Makefile > client/Makefile.out
        mv client/Makefile.out client/Makefile
    fi
    if [ "$VERBOSE" = "no" ]; then
        make --silent
        make stage --silent
    else
        make SHELL="/bin/bash -x"
        make stage SHELL="/bin/bash -x"
    fi

    echo "Stripping Binaries"
    cd stage/usr/local/bin
    x86_64-linux-android-strip *
    cd ../../../../

    echo "Copy Assets"
    cd android
    mkdir -p "BOINC/app/src/main/assets"
    cp "$BOINC/stage/usr/local/bin/boinc" "BOINC/app/src/main/assets/x86_64/boinc"
    cp "$BOINC/win_build/installerv2/redist/all_projects_list.xml" "BOINC/app/src/main/assets/all_projects_list.xml"
    cp "$BOINC/curl/ca-bundle.crt" "BOINC/app/src/main/assets/ca-bundle.crt"

    echo "===== BOINC for x86-64 build done ====="

fi
