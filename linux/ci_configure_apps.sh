#!/bin/sh
set -e

if [ ! -d "linux" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

BUILD_DIR="$PWD/3rdParty/linux"
VCPKG_ROOT="$BUILD_DIR/vcpkg"
export VCPKG_DIR="$VCPKG_ROOT/installed/x64-linux"

linux/update_vcpkg_apps.sh

export _libcurl_pc="$VCPKG_DIR/lib/pkgconfig/libcurl.pc"
export PKG_CONFIG_PATH=$VCPKG_DIR/lib/pkgconfig/
export X_EXTRA_LIBS="-I$VCPKG_DIR/include $(pkg-config --libs freeglut)"

./configure --with-libcurl=$VCPKG_DIR --with-ssl=$VCPKG_DIR --enable-apps --enable-apps-vcpkg --enable-apps-vbox --enable-apps-gui --disable-server --disable-client --disable-manager CPPFLAGS="-I$VCPKG_DIR/include"
