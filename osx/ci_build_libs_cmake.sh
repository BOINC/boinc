#!/bin/sh
set -e

PLATFORM_NAME="osx"

if [ ! -d "$PLATFORM_NAME" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

BUILD_DIR="$PWD/3rdParty/$PLATFORM_NAME"
VCPKG_ROOT="$BUILD_DIR/vcpkg"
VCPKG_PORTS="$PWD/3rdParty/vcpkg_ports"

$PLATFORM_NAME/bootstrap_vcpkg_cmake.sh

TRIPLETS_LIST="x64-osx arm64-osx"

for TRIPLET in $TRIPLETS_LIST ; do
    echo "\033[0;35m building $TRIPLET ... \033[0m"

    BUILD_TRIPLET=build-$TRIPLET
    cmake lib -B $BUILD_TRIPLET -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DVCPKG_MANIFEST_DIR=3rdParty/vcpkg_ports/configs/libs/ -DVCPKG_INSTALLED_DIR=$VCPKG_ROOT/installed/ -DVCPKG_OVERLAY_PORTS=$VCPKG_PORTS/ports -DVCPKG_OVERLAY_TRIPLETS=$VCPKG_PORTS/triplets/ci -DVCPKG_TARGET_TRIPLET=$TRIPLET -DVCPKG_INSTALL_OPTIONS=--clean-after-build
    cmake --build $BUILD_TRIPLET

    echo "\033[1;32m $TRIPLET done \033[0m"
done
