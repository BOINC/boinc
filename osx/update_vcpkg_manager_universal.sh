#!/bin/sh
set -e

if [ ! -d "osx" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

source $PWD/3rdParty/vcpkg_ports/vcpkg_link.sh
BUILD_DIR="$PWD/3rdParty/osx"
VCPKG_PORTS="$PWD/3rdParty/vcpkg_ports"
VCPKG_ROOT="$BUILD_DIR/vcpkg"

if [ ! -d $VCPKG_ROOT ]; then
    mkdir -p $BUILD_DIR
    git -C $BUILD_DIR clone $VCPKG_LINK
fi

git -C $VCPKG_ROOT pull
$VCPKG_ROOT/bootstrap-vcpkg.sh

# delete prev custom ports
rm -rf $VCPKG_PORTS/ports/mac/*

# create custom ports base on latest ports with patches
cp -R $VCPKG_ROOT/ports/wxwidgets $VCPKG_PORTS/ports/mac
cp $VCPKG_PORTS/patches/boinc_SetItemBitmap.patch $VCPKG_PORTS/patches/boinc_SetVisibilityHidden.patch  $VCPKG_PORTS/ports/mac/wxwidgets

# apply patches
patch -p1 -d $VCPKG_PORTS/ports/mac -i $VCPKG_PORTS/patches/wxwidgets.patch

$VCPKG_ROOT/vcpkg install  --x-manifest-root=3rdParty/vcpkg_ports/configs/manager/osx --x-install-root=$VCPKG_ROOT/installed/arm64/ --overlay-ports=$VCPKG_PORTS/ports/mac --overlay-triplets=$VCPKG_PORTS/triplets/ci --triplet=arm64-osx --clean-after-build
$VCPKG_ROOT/vcpkg install  --x-manifest-root=3rdParty/vcpkg_ports/configs/manager/osx --x-install-root=$VCPKG_ROOT/installed/x64/   --overlay-ports=$VCPKG_PORTS/ports/mac --overlay-triplets=$VCPKG_PORTS/triplets/ci --triplet=x64-osx   --clean-after-build
