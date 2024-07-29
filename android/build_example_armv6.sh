#/bin/sh
set -e

#script to compile Example for Android

STDOUT_TARGET="${STDOUT_TARGET:-/dev/stdout}"
COMPILEBOINC="yes"
CONFIGURE="yes"
VERBOSE="${VERBOSE:-no}"
NPROC_USER="${NPROC_USER:-1}"

export BOINC="$(pwd)/.." #BOINC source code

export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROIDTC="${ANDROID_TC_ARMV6:-$ANDROID_TC/armv6}"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/arm-linux-androideabi"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib/libstdc++.a"
export BOINC_API_DIR="$BOINC/api"
export BOINC_LIB_DIR="$BOINC/lib"
export BOINC_ZIP_DIR="$BOINC/zip"
export VCPKG_DIR=$VCPKG_ROOT/installed/armv6-android

CONFIG_FLAGS=""
CONFIG_LDFLAGS=""

CONFIG_LDFLAGS="-L$VCPKG_DIR/lib"
CONFIG_FLAGS="--with-ssl=$VCPKG_DIR --with-libcurl=$VCPKG_DIR --enable-apps-vcpkg"
export _libcurl_pc="$VCPKG_DIR/lib/pkgconfig/libcurl.pc"

export ANDROID="yes"
export PATH="$TCBINARIES:$TCINCLUDES/bin:$PATH"
export CC=arm-linux-androideabi-clang
export CXX=arm-linux-androideabi-clang++
export LD=arm-linux-androideabi-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -DDECLARE_TIMEZONE -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE -march=armv6 -mfloat-abi=softfp -mfpu=vfp -D__ANDROID_API__=16 -DARMV6 -I$TCINCLUDES/include -I$BOINC -I$BOINC_LIB_DIR -I$BOINC_API_DIR -I$BOINC_ZIP_DIR"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall  -funroll-loops -fexceptions -O3 -fomit-frame-pointer -I$TCINCLUDES/include -fPIE -march=armv6 -mfloat-abi=softfp -mfpu=vfp -D__ANDROID_API__=16 -DARMV6 -I$BOINC -I$BOINC_LIB_DIR -I$BOINC_API_DIR -I$BOINC_ZIP_DIR"
export LDFLAGS="$CONFIG_LDFLAGS -L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog -fPIE -pie -latomic -static-libstdc++ -march=armv6"
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
    cd $BOINC
    echo "===== building example for armv6 from $PWD ====="
    if [ -n "$MAKECLEAN" ] && [ -f "Makefile" ]; then
        echo "=== building example clean ==="
        if [ "$VERBOSE" = "no" ]; then
            make distclean 1>$STDOUT_TARGET 2>&1
        else
            make distclean SHELL="/bin/bash -x"
        fi
    fi
    if [ -n "$CONFIGURE" ]; then
        ./_autosetup
        ./configure --host=armv6-linux --with-boinc-platform="armv-android-linux-gnu" --prefix="$TCINCLUDES" --libdir="$TCINCLUDES/lib" $CONFIG_FLAGS --enable-apps --disable-server --disable-manager --disable-client --disable-libraries --disable-shared --enable-static --disable-largefile --enable-boinczip
    fi
    echo MAKE_FLAGS=$MAKE_FLAGS
    make $MAKE_FLAGS
    echo "\e[1;32m===== building example for armv6 done =====\e[0m"
fi
