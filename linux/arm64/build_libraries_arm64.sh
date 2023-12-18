#!/bin/sh
set -e

# Script to compile various BOINC libraries for linux arm64

STDOUT_TARGET="${STDOUT_TARGET:-/dev/stdout}"
COMPILEBOINC="yes"
CONFIGURE="yes"
MAKECLEAN="yes"
VERBOSE="${VERBOSE:-no}"
CI="${CI:-no}"
NPROC_USER="${NPROC_USER:-1}"
RELEASE="${RELEASE:-no}"

export BUILD_DIR=${BUILD_DIR:-$PWD/3rdParty/linux-arm64}

export TCINCLUDES="$BUILD_DIR/build"
export TCSYSROOT="$BUILD_DIR/sysroot"

export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++
export LD=aarch64-linux-gnu-ld
export CFLAGS="--sysroot=$TCSYSROOT -march=armv8-a -O3"
export CXXFLAGS="--sysroot=$TCSYSROOT -march=armv8-a -O3"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -march=armv8-a -static-libstdc++ -static"

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

    echo "===== building BOINC Libraries for arm64 from $PWD ====="
    if [ -n "$MAKECLEAN" ] && [ -f "Makefile" ]; then
        if [ "$VERBOSE" = "no" ]; then
            make distclean 1>$STDOUT_TARGET 2>&1
        else
            make distclean SHELL="/bin/bash -x"
        fi
    fi
    if [ -n "$CONFIGURE" ]; then
        ./_autosetup
        ./configure --host=aarch64-linux --with-boinc-platform="aarch64-unknown-linux-gnu" --with-boinc-alt-platform="arm-unknown-linux-gnueabihf" --prefix="$TCINCLUDES" --libdir="$TCINCLUDES/lib" --disable-server --disable-manager --disable-client --disable-shared --enable-static --enable-boinczip
    fi
    echo MAKE_FLAGS=$MAKE_FLAGS
    make $MAKE_FLAGS
    make stage $MAKE_FLAGS
    make install $MAKE_FLAGS

    echo "\e[1;32m===== building BOINC Libraries for arm64 done =====\e[0m"

fi
