#!/bin/sh
set -e

if [ ! -d "osx" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

CACHE_DIR="$PWD/3rdParty/buildCache/mac"
BUILD_DIR="$PWD/3rdParty/osx"
VCPKG_ROOT="$BUILD_DIR/vcpkg"
VCPKG_X64="$VCPKG_ROOT/installed/x64/x64-osx"
VCPKG_ARM64="$VCPKG_ROOT/installed/arm64/arm64-osx"
MAC_VCPKG=$BUILD_DIR/mac_vcpkg

osx/update_vcpkg_manager_universal.sh

echo " "
echo "Copy includes"
mkdir -p "$MAC_VCPKG/lib"
cp -R $VCPKG_X64/include $MAC_VCPKG
find $VCPKG_X64/lib/* -type d -maxdepth 0 -not -name "pkgconfig" -exec cp -R {} "$MAC_VCPKG/lib" \;

echo "Create universal libs:"

for lib_x64 in $VCPKG_X64/lib/*.a; do
    lib_full_name=$(basename $lib_x64)
    lib_name=$(basename -s .a $lib_x64)
    lib_arm64=""
    lib_universal="$MAC_VCPKG/lib/$lib_full_name"
    if   [ -f "$VCPKG_ARM64/lib/$lib_full_name" ]; then
            lib_arm64="$VCPKG_ARM64/lib/$lib_full_name"
    elif [ -f "$VCPKG_ARM64/lib/$lib_name-Darwin.a" ]; then
            lib_arm64="$VCPKG_ARM64/lib/$lib_name-Darwin.a"
    fi
    if [ ! -z $lib_arm64 ]; then
        echo exist $lib_full_name
        lipo -create  "$lib_x64" "$lib_arm64" -output $lib_universal
        if ! lipo "$lib_x64" -verify_arch x86_64; then
            echo "Fail verify x86_64 on $lib_x64"
            exit 1
        fi
        if ! lipo "$lib_arm64" -verify_arch arm64; then
            echo "Fail verify arm64 on $lib_arm64"
            exit 1
        fi
        if ! lipo "$lib_universal" -verify_arch x86_64 arm64; then
            echo "Fail verify x86_64 arm64 on $lib_universal"
            exit 1
        fi
    else
        echo Not exist $lib_full_name
        exit 1
    fi
done
