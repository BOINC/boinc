#!/bin/sh
set -e

# Script to compile Libcurl for linux arm64

STDOUT_TARGET="${STDOUT_TARGET:-/dev/stdout}"
CONFIGURE="yes"
MAKECLEAN="yes"
VERBOSE="${VERBOSE:-no}"
CI="${CI:-no}"
NPROC_USER="${NPROC_USER:-1}"

export CURL_VERSION=7.84.0
export BUILD_DIR=${BUILD_DIR:-$PWD/3rdParty/linux-arm64}
export CURL="$BUILD_DIR/curl-$CURL_VERSION" #CURL sources, required by BOINC
export CURL_FLAGFILE=$BUILD_DIR/curl-$CURL_VERSION.flagfile

export TCINCLUDES="$BUILD_DIR/build"
export TCBINARIES="$TCINCLUDES/bin"
export TCSYSROOT="$BUILD_DIR/sysroot"

export PATH="$TCBINARIES:$PATH"
export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++
export LD=aarch64-linux-gnu-ld
export CFLAGS="--sysroot=$TCSYSROOT -I$TCINCLUDES/include -march=armv8-a -O3"
export CXXFLAGS="--sysroot=$TCSYSROOT -I$TCINCLUDES/include -march=armv8-a -O3"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -march=armv8-a -latomic -static-libstdc++"

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

if [ ! -e "${CURL_FLAGFILE}" ]; then
    rm -rf "$BUILD_DIR/curl-${CURL_VERSION}"
    wget -c --no-verbose -O /tmp/curl_${CURL_VERSION}.tgz https://curl.haxx.se/download/curl-${CURL_VERSION}.tar.gz
    tar xzf /tmp/curl_${CURL_VERSION}.tgz --directory=$BUILD_DIR
fi

if [ ! -e "${CURL_FLAGFILE}" ]; then
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
        ./configure --host=aarch64-linux --prefix="$TCINCLUDES" --libdir="$TCINCLUDES/lib" --disable-shared --enable-static --with-random=/dev/urandom 1>$STDOUT_TARGET --with-openssl
    fi
    if [ $VERBOSE = "no" ]; then
        echo MAKE_FLAGS=$MAKE_FLAGS "1>$STDOUT_TARGET"
        make $MAKE_FLAGS 1>$STDOUT_TARGET
        make install $MAKE_FLAGS 1>$STDOUT_TARGET
    else
        echo MAKE_FLAGS=$MAKE_FLAGS
        make $MAKE_FLAGS
        make install $MAKE_FLAGS
    fi

    touch "${CURL_FLAGFILE}"
    echo "\e[1;32m===== curl for arm64 build done =====\e[0m"
fi
