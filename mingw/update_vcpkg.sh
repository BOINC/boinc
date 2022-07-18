#!/bin/sh
set -e

if [ ! -d "mingw" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

. $PWD/3rdParty/vcpkg_ports/vcpkg_link.sh
CACHE_DIR="$PWD/3rdParty/buildCache/mingw"
BUILD_DIR="$PWD/3rdParty/mingw"
VCPKG_PORTS="$PWD/3rdParty/vcpkg_ports"
VCPKG_ROOT="$BUILD_DIR/vcpkg"

if [ ! -d $VCPKG_ROOT ]; then
    mkdir -p $BUILD_DIR
    git -C $BUILD_DIR clone $VCPKG_LINK
fi

git -C $VCPKG_ROOT pull
$VCPKG_ROOT/bootstrap-vcpkg.sh
$VCPKG_ROOT/vcpkg install --x-manifest-root=3rdParty/vcpkg_ports/configs/apps/mingw --x-install-root=$VCPKG_ROOT/installed/ --overlay-ports=$VCPKG_PORTS/ports --overlay-triplets=$VCPKG_PORTS/triplets/ci --triplet=x64-mingw-static --clean-after-build
