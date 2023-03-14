#!/bin/sh
set -e

PLATFORM_NAME="android"

if [ ! -d "$PLATFORM_NAME" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

BUILD_DIR="$PWD/3rdParty/$PLATFORM_NAME"
VCPKG_ROOT="$BUILD_DIR/vcpkg"
VCPKG_PORTS="$PWD/3rdParty/vcpkg_ports"
. $PLATFORM_NAME/ndk_common.sh

if [ ! -d $NDK_ROOT ]; then
    createNDKFolder
fi

if [ ! -d $NDK_ARMV6_ROOT ]; then
    createNDKARMV6Folder
fi

$PLATFORM_NAME/bootstrap_vcpkg_cmake.sh
./_autosetup

arch=""

while [ $# -gt 0 ]; do
    key="$1"
    case $key in
        --arch)
        arch="$2"
        shift
        ;;
        *)
        echo "unrecognized option $key"
        ;;
    esac
    shift # past argument or value
done

if [ "x$arch" = "x" ]; then
    TRIPLETS_LIST="armv6-android arm-android arm-neon-android arm64-android x86-android x64-android"
elif [ "x$arch" = "xarm" ]; then
    TRIPLETS_LIST="arm-android arm-neon-android"
elif [ "x$arch" = "x86_64" ]; then
    TRIPLETS_LIST="x64-android"
else
    TRIPLETS_LIST="$arch-android"
fi

for TRIPLET in $TRIPLETS_LIST ; do
    echo "\e[0;35m building $TRIPLET ... \e[0m"

    if [ "$TRIPLET" = "armv6-android" ]; then
        export ANDROID_NDK_HOME=$NDK_ARMV6_ROOT
    else
        export ANDROID_NDK_HOME=$NDK_ROOT
    fi
    BUILD_TRIPLET=build-$TRIPLET
    cmake lib -B $BUILD_TRIPLET -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DVCPKG_MANIFEST_DIR=3rdParty/vcpkg_ports/configs/libs/ -DVCPKG_MANIFEST_INSTALL=$VCPKG_ROOT/installed/ -DVCPKG_OVERLAY_PORTS=$VCPKG_PORTS/ports -DVCPKG_OVERLAY_TRIPLETS=$VCPKG_PORTS/triplets/ci -DVCPKG_TARGET_TRIPLET=$TRIPLET -DVCPKG_INSTALL_OPTIONS=--clean-after-build
    cmake --build $BUILD_TRIPLET

    echo "\e[1;32m $TRIPLET done \e[0m"
done
