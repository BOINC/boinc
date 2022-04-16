#!/bin/sh
set -e

PLATFORM_NAME="android"

if [ ! -d "$PLATFORM_NAME" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

BUILD_DIR="$PWD/3rdParty/$PLATFORM_NAME"
VCPKG_ROOT="$BUILD_DIR/vcpkg"

if [ ! -d $VCPKG_ROOT ]; then
    mkdir -p $BUILD_DIR
    git -C $BUILD_DIR clone https://github.com/microsoft/vcpkg
fi

git -C $VCPKG_ROOT pull
$VCPKG_ROOT/bootstrap-vcpkg.sh
