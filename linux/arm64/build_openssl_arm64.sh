#!/bin/sh
set -e

# Script to compile OpenSSL for linux arm64

STDOUT_TARGET="${STDOUT_TARGET:-/dev/stdout}"
CONFIGURE="yes"
MAKECLEAN="yes"
VERBOSE="${VERBOSE:-no}"
CI="${CI:-no}"
NPROC_USER="${NPROC_USER:-1}"
RELEASE="${RELEASE:-no}"

export OPENSSL_VERSION=3.2.0
export BUILD_DIR=${BUILD_DIR:-$PWD/3rdParty/linux-arm64}
export OPENSSL="$BUILD_DIR/openssl-$OPENSSL_VERSION" #openSSL sources, required by BOINC
export OPENSSL_FLAGFILE=$BUILD_DIR/openssl-$OPENSSL_VERSION.flagfile

export TCINCLUDES="$BUILD_DIR/build"
export TCBINARIES="$TCINCLUDES/bin"
export TCSYSROOT="$BUILD_DIR/sysroot"

export PATH="$TCBINARIES:$PATH"
export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++
export LD=aarch64-linux-gnu-ld
export CFLAGS="--sysroot=$TCSYSROOT -march=armv8-a -O3"
export CXXFLAGS="--sysroot=$TCSYSROOT -march=armv8-a -O3"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -march=armv8-a -latomic -static-libstdc++"

MAKE_FLAGS=""

if [ $VERBOSE = "no" ]; then
    MAKE_FLAGS="$MAKE_FLAGS --silent"
else
    MAKE_FLAGS="$MAKE_FLAGS SHELL=\"/bin/bash -x\""
fi

if [ $CI = "yes" ]; then
    MAKE_FLAGS1="$MAKE_FLAGS -j $(nproc --all)"
else
    MAKE_FLAGS1="$MAKE_FLAGS -j $NPROC_USER"
fi

if [ $RELEASE = "yes" ]; then
    LDFLAGS="$LDFLAGS -s"
fi

mkdir -p $BUILD_DIR

if [ ! -e "${OPENSSL_FLAGFILE}" ]; then
    rm -rf "$BUILD_DIR/openssl-${OPENSSL_VERSION}"
    wget -c --no-verbose -O /tmp/openssl_${OPENSSL_VERSION}.tgz https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz
    tar xzf /tmp/openssl_${OPENSSL_VERSION}.tgz --directory=$BUILD_DIR
fi

if [ ! -e "${OPENSSL_FLAGFILE}"  ]; then
    cd "$OPENSSL"
    echo "===== building openssl for arm64 from $PWD ====="
    if [ -n "$MAKECLEAN" -a -e "${OPENSSL}/Makefile" ]; then
        if [ "$VERBOSE" = "no" ]; then
            make clean 1>$STDOUT_TARGET 2>&1
        else
            make clean SHELL="/bin/bash -x"
        fi
    fi
    if [ -n "$CONFIGURE" ]; then
        ./Configure linux-generic32 no-shared no-dso -DL_ENDIAN --openssldir="$TCINCLUDES" --prefix="$TCINCLUDES" 1>$STDOUT_TARGET
    fi
    if [ $VERBOSE = "no" ]; then
        echo MAKE_FLAGS=$MAKE_FLAGS "1>$STDOUT_TARGET"
        make $MAKE_FLAGS 1>$STDOUT_TARGET
        make install_sw $MAKE_FLAGS 1>$STDOUT_TARGET
    else
        echo MAKE_FLAGS=$MAKE_FLAGS
        make $MAKE_FLAGS
        make install_sw $MAKE_FLAGS
    fi

    touch "${OPENSSL_FLAGFILE}"
    echo "\e[1;32m===== openssl for arm64 build done =====\e[0m"
fi
