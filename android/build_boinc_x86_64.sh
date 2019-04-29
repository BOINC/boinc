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

export BOINC=".." #BOINC source code

export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROIDTC="${ANDROID_TC_X86_64-$ANDROID_TC/x86_64}"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/x86_64-linux-android"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib64/libstdc++.a"

export PATH="$TCBINARIES:$TCINCLUDES/bin:$PATH"
export CC=x86_64-linux-android-clang
export CXX=x86_64-linux-android-clang++
export LD=x86_64-linux-android-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -DANDROID_64 -DDECLARE_TIMEZONE -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE -D__ANDROID_API__=21"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -DANDROID_64 -Wall -I$TCINCLUDES/include -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE -D__ANDROID_API__=21"
export LDFLAGS="-L$TCSYSROOT/usr/lib64 -L$TCINCLUDES/lib64 -llog -fPIE -pie -latomic -static-libstdc++"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"
export PKG_CONFIG_SYSROOT_DIR="$TCSYSROOT"

# Prepare android toolchain and environment
./build_androidtc_x86_64.sh

if [ -n "$COMPILEBOINC" ]; then
    echo "===== building BOINC for x86-64 from $BOINC ====="
    cd "$BOINC"
    if [ -n "$MAKECLEAN" ] && [ -f "Makefile" ]; then
        make distclean 1>$STDOUT_TARGET 2>&1
    fi
    if [ -n "$CONFIGURE" ]; then
        ./_autosetup
        ./configure --host=x86_64-linux --with-boinc-platform="x86_64-android-linux-gnu" --with-boinc-alt-platform="x86-android-linux-gnu" --with-ssl="$TCINCLUDES" --disable-server --disable-manager --disable-shared --enable-static
        sed -e "s%^CLIENTLIBS *= *.*$%CLIENTLIBS = -lm $STDCPPTC%g" client/Makefile > client/Makefile.out
        mv client/Makefile.out client/Makefile
    fi
    make --silent
    make stage --silent

    echo "Stripping Binaries"
    cd stage/usr/local/bin
    x86_64-linux-android-strip *
    cd ../../../../

    echo "Copy Assets"
    cd android
    mkdir -p "BOINC/app/src/main/assets"
    cp "$BOINC/stage/usr/local/bin/boinc" "BOINC/app/src/main/assets/x86_64/boinc"
    cp "$BOINC/stage/usr/local/bin/boinccmd" "BOINC/app/src/main/assets/x86_64/boinccmd"
    cp "$BOINC/win_build/installerv2/redist/all_projects_list.xml" "BOINC/app/src/main/assets/all_projects_list.xml"
    cp "$BOINC/curl/ca-bundle.crt" "BOINC/app/src/main/assets/ca-bundle.crt"

    echo "===== BOINC for x86-64 build done ====="

fi
