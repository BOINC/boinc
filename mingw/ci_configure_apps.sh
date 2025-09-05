#!/bin/sh
set -e

if [ ! -d "mingw" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

CACHE_DIR="$PWD/3rdParty/buildCache/mingw"
BUILD_DIR="$PWD/3rdParty/mingw"
VCPKG_ROOT="$BUILD_DIR/vcpkg"
export VCPKG_DIR="$VCPKG_ROOT/installed/x64-mingw-static"

mingw/update_vcpkg.sh

export CXXFLAGS="-I$VCPKG_DIR/include -L$VCPKG_DIR/lib"
export CFLAGS="$CXXFLAGS"

export PKG_CONFIG_PATH=$VCPKG_DIR/lib/pkgconfig/
export _libcurl_pc="$VCPKG_DIR/lib/pkgconfig/libcurl.pc"
./configure --host=x86_64-w64-mingw32 --with-libcurl=$VCPKG_DIR --with-ssl=$VCPKG_DIR --enable-apps --enable-apps-mingw --enable-apps-vcpkg --enable-apps-gui --disable-server --disable-client --disable-manager
