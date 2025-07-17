#!/bin/sh
set -e

if [ ! -d "linux" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

BUILD_DIR="$PWD/3rdParty/linux"
VCPKG_ROOT="$BUILD_DIR/vcpkg"
export VCPKG_DIR="$VCPKG_ROOT/installed/arm64-linux-release"

linux/arm64/update_vcpkg_libs.sh

export _libcurl_pc="$VCPKG_DIR/lib/pkgconfig/libcurl.pc"
./configure --host=aarch64-linux-gnu --with-boinc-platform="aarch64-unknown-linux-gnu" --with-boinc-alt-platform="arm-unknown-linux-gnueabihf" --with-libcurl=$VCPKG_DIR --with-ssl=$VCPKG_DIR --disable-server --disable-client --disable-manager CPPFLAGS="-I$VCPKG_DIR/include"
