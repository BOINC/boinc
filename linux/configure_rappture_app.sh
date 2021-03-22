#!/bin/sh
set -e

if [ ! -d "linux" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

BUILD_DIR="$PWD/3rdParty/buildCache"

if [ ! -f "$BUILD_DIR/vcpkg/installed/x64-linux/lib/librappture.a" ]; then
    rm -rf "$BUILD_DIR/vcpkg"
    mkdir -p $BUILD_DIR
    git -C $BUILD_DIR clone https://github.com/microsoft/vcpkg
    $BUILD_DIR/vcpkg/bootstrap-vcpkg.sh
    $BUILD_DIR/vcpkg/vcpkg install rappture
fi

export RAPPTURE="yes"
./configure --enable-apps --disable-server --disable-client --disable-manager
