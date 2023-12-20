#/bin/sh
set -e

#script to compile Example for Android

STDOUT_TARGET="${STDOUT_TARGET:-/dev/stdout}"
COMPILEBOINC="yes"
CONFIGURE="yes"
VERBOSE="${VERBOSE:-no}"
CI="${CI:-no}"
NPROC_USER="${NPROC_USER:-1}"
RELEASE="${RELEASE:-no}"

export BUILD_DIR=${BUILD_DIR:-$PWD/3rdParty/linux-arm64}

export TCINCLUDES="$BUILD_DIR/build"
export TCBINARIES="$TCINCLUDES/bin"
export TCSYSROOT="$BUILD_DIR/sysroot"

export BOINC_API_DIR="$PWD/api"
export BOINC_LIB_DIR="$PWD/lib"
export BOINC_ZIP_DIR="$PWD/zip"

export PATH="$TCBINARIES:$PATH"
export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++
export LD=aarch64-linux-gnu-ld
export CFLAGS="--sysroot=$TCSYSROOT -I$TCINCLUDES/include -march=armv8-a -O3 -I$BOINC -I$BOINC_LIB_DIR -I$BOINC_API_DIR -I$BOINC_ZIP_DIR"
export CXXFLAGS="--sysroot=$TCSYSROOT -I$TCINCLUDES/include -march=armv8-a -O3 -I$BOINC -I$BOINC_LIB_DIR -I$BOINC_API_DIR -I$BOINC_ZIP_DIR"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -march=armv8-a -latomic -static-libstdc++"

CONFIG_FLAGS="--with-ssl=$TCINCLUDES --with-libcurl=$TCINCLUDES"
CONFIG_LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib"
export _libcurl_pc="$TCINCLUDES/lib/pkgconfig/libcurl.pc"

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

if [ $RELEASE = "yes" ]; then
    LDFLAGS="$LDFLAGS -s"
fi

if [ -n "$COMPILEBOINC" ]; then
    echo "===== building example for arm64 from $PWD ====="
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
        ./configure --host=aarch64-linux --with-boinc-platform="aarch64-unknown-linux-gnu" --with-boinc-alt-platform="arm-unknown-linux-gnueabihf" --prefix="$TCINCLUDES" --libdir="$TCINCLUDES/lib" $CONFIG_FLAGS --enable-apps --disable-server --disable-manager --disable-client --disable-libraries --disable-shared --enable-static --enable-boinczip
    fi
    echo MAKE_FLAGS=$MAKE_FLAGS
    make $MAKE_FLAGS
    echo "\e[1;32m===== building example for arm64 done =====\e[0m"
fi
