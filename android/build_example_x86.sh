#/bin/sh
set -e

#script to compile Example for Android

STDOUT_TARGET="${STDOUT_TARGET:-/dev/stdout}"
COMPILEBOINC="yes"
CONFIGURE="yes"
VERBOSE="${VERBOSE:-no}"
NPROC_USER="${NPROC_USER:-1}"

export BOINC="$(pwd)/.." #BOINC source code

export NDK_ROOT=${NDK_ROOT:-$HOME/Android/Ndk}
export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROIDTC="${ANDROID_TC_X86:-$ANDROID_TC/x86}"
export TOOLCHAINROOT="$NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64"
export TCBINARIES="$TOOLCHAINROOT/bin"
export TCINCLUDES="$ANDROIDTC/i686-linux-android"
export TCSYSROOT="$TOOLCHAINROOT/sysroot"
export BOINC_API_DIR="$BOINC/api"
export BOINC_LIB_DIR="$BOINC/lib"
export BOINC_ZIP_DIR="$BOINC/zip"
export VCPKG_DIR=$VCPKG_ROOT/installed/x86-android

CONFIG_FLAGS=""
CONFIG_LDFLAGS=""

if [ $BUILD_WITH_VCPKG = "yes" ]; then
    CONFIG_CFLAGS="-I$VCPKG_DIR/include"
    CONFIG_CXXFLAGS="-I$VCPKG_DIR/include"
    CONFIG_LDFLAGS="-L$VCPKG_DIR/lib"
    CONFIG_FLAGS="--with-ssl=$VCPKG_DIR --enable-vcpkg"
else
    CONFIG_CFLAGS="-I$TCINCLUDES/include"
    CONFIG_CXXFLAGS="-I$TCINCLUDES/include"
    CONFIG_LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib"
    CONFIG_FLAGS="--with-ssl=$TCINCLUDES"
fi

export ANDROID="yes"
export PATH="$TCBINARIES:$TCINCLUDES/bin:$PATH"
export CC=i686-linux-android16-clang
export CXX=i686-linux-android16-clang++
export LD=i686-linux-android-ld
export CFLAGS="--sysroot=$TCSYSROOT $CONFIG_CFLAGS -DANDROID -DDECLARE_TIMEZONE -Wall -O3 -fomit-frame-pointer -fPIE -D__ANDROID_API__=16 -I$BOINC -I$BOINC_LIB_DIR -I$BOINC_API_DIR -I$BOINC_ZIP_DIR"
export CXXFLAGS="--sysroot=$TCSYSROOT $CONFIG_CXXFLAGS -DANDROID -Wall -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE -D__ANDROID_API__=16 -I$BOINC -I$BOINC_LIB_DIR -I$BOINC_API_DIR -I$BOINC_ZIP_DIR"
export LDFLAGS="$CONFIG_LDFLAGS -L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog -fPIE -pie -latomic -static-libstdc++ -lz"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g"

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
    echo "===== building example for x86 from $PWD ====="
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
        ./configure --host=i686-linux --with-boinc-platform="x86-android-linux-gnu" --prefix="$TCINCLUDES" --libdir="$TCINCLUDES/lib" $CONFIG_FLAGS --enable-apps --disable-server --disable-manager --disable-client --disable-libraries --disable-shared --enable-static --disable-largefile --enable-boinczip
    fi
    echo MAKE_FLAGS=$MAKE_FLAGS
    make $MAKE_FLAGS
    echo "\e[1;32m===== building example for x86 done =====\e[0m"
fi
