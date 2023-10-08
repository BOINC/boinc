#!/bin/sh
set -e

#
# See: https://github.com/BOINC/boinc/wiki/AndroidBuildClient
#

# Script to compile BOINC selected component for Android

# check working directory because the script needs to be called like: ./android/ci_build_apps.sh
if [ ! -d "android" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

ci=""
component=""
arch=""

while [ $# -gt 0 ]; do
    key="$1"
    case $key in
        --ci)
        ci="$key"
        ;;
        --component)
        component=$2
        shift
        ;;
        --arch)
        arch=$2
        shift
        ;;
        *)
        echo "unrecognized option $key"
        ;;
    esac
    shift # past argument or value
done

if [ "x$component" = "x" ]; then
    echo "No component to build"
    exit 1
fi

cd android

rm -rf /tmp/vcpkg_updated

if [ "x$arch" = "x" ]; then
    ./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --silent $ci --component $component --arch armv6
    ./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --silent $ci --component $component --arch arm
    ./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --silent $ci --component $component --arch arm64
    ./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --silent $ci --component $component --arch x86
    ./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --silent $ci --component $component --arch x86_64
else
    ./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --silent $ci --component $component --arch $arch
fi

cd ..
