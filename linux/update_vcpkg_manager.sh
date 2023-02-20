#!/bin/sh
set -e

if [ ! -d "linux" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

TRIPLET="x64-linux"
if [ ! -z "$1" ]; then
    TRIPLET="$1"
fi

echo TRIPLET=$TRIPLET

. $PWD/3rdParty/vcpkg_ports/vcpkg_link.sh
BUILD_DIR="$PWD/3rdParty/linux"
VCPKG_PORTS="$PWD/3rdParty/vcpkg_ports"
VCPKG_ROOT="$BUILD_DIR/vcpkg"

if [ ! -d $VCPKG_ROOT ]; then
    mkdir -p $BUILD_DIR
    git -C $BUILD_DIR clone $VCPKG_LINK
fi

git -C $VCPKG_ROOT pull
$VCPKG_ROOT/bootstrap-vcpkg.sh
$VCPKG_ROOT/vcpkg install  --x-manifest-root=3rdParty/vcpkg_ports/configs/manager/linux --x-install-root=$VCPKG_ROOT/installed/ --overlay-ports=$VCPKG_PORTS/ports --overlay-triplets=$VCPKG_PORTS/triplets/ci --triplet=$TRIPLET --clean-after-build
