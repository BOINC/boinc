#!/bin/sh
set -e

if [ ! -d "osx" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

. $PWD/3rdParty/vcpkg_ports/vcpkg_link.sh
BUILD_DIR="$PWD/3rdParty/osx"
VCPKG_PORTS="$PWD/3rdParty/vcpkg_ports"
VCPKG_ROOT="$BUILD_DIR/vcpkg"

if [ ! -d $VCPKG_ROOT ]; then
    mkdir -p $BUILD_DIR
    git -C $BUILD_DIR clone $VCPKG_LINK
fi

git -C $VCPKG_ROOT pull
$VCPKG_ROOT/bootstrap-vcpkg.sh
$VCPKG_ROOT/vcpkg install curl[core,openssl] wxwidgets[core] ftgl --triplet=arm64-osx --clean-after-build --overlay-triplets=$VCPKG_PORTS/triplets/ci
