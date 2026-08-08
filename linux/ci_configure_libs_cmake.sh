#!/bin/sh
set -e

if [ ! -d "linux" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

BUILD_DIR="$PWD/3rdParty/linux"
VCPKG_ROOT="$BUILD_DIR/vcpkg"
VCPKG_PORTS="$PWD/3rdParty/vcpkg_ports"

export VCPKG_DIR="$VCPKG_ROOT/installed/x64-linux"

linux/bootstrap_vcpkg_cmake.sh

cmake lib -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DVCPKG_MANIFEST_DIR=3rdParty/vcpkg_ports/configs/libs/ -DVCPKG_INSTALLED_DIR=$VCPKG_ROOT/installed/ -DVCPKG_OVERLAY_PORTS=$VCPKG_PORTS/ports -DVCPKG_OVERLAY_TRIPLETS=$VCPKG_PORTS/triplets/ci -DVCPKG_TARGET_TRIPLET=x64-linux -DVCPKG_INSTALL_OPTIONS=--clean-after-build
