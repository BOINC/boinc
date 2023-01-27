#!/bin/sh
set -e

if [ ! -d "linux" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

CACHE_DIR="$PWD/3rdParty/buildCache/mac"
BUILD_DIR="$PWD/3rdParty/osx"
VCPKG_ROOT="$BUILD_DIR/vcpkg"
VCPKG_X64="$VCPKG_ROOT/installed/x64-osx/lib"
VCPKG_ARM64="$VCPKG_ROOT/installed/arm64-osx/lib"

osx/update_vcpkg_manager_x64.sh
osx/update_vcpkg_manager_arm64.sh

for lib in $VCPKG_X64; do
    echo $lib;
done

for lib in $VCPKG_ARM64; do
    echo $lib;
done
