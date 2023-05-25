#!/bin/sh
set -e

if [ ! -d "linux" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

CACHE_DIR="$PWD/3rdParty/buildCache/linux"
BUILD_DIR="$PWD/3rdParty/linux"
VCPKG_ROOT="$BUILD_DIR/vcpkg"
export VCPKG_DIR="$VCPKG_ROOT/installed/x64-linux"

linux/update_vcpkg_manager.sh

export _libcurl_pc="$VCPKG_DIR/lib/pkgconfig/libcurl.pc"
export PKG_CONFIG_PATH=$VCPKG_DIR/lib/pkgconfig/

./configure --enable-vcpkg --disable-server --disable-client --with-wx-config=$VCPKG_DIR/tools/wxwidgets/wx-config CPPFLAGS="-DwxDEBUG_LEVEL=0" GTK_LIBS="$(pkg-config --libs gtk+-3.0)"
