#!/bin/sh
set -e

#
# See: https://boinc.berkeley.edu/trac/wiki/AndroidBuildClient
#

# Script to compile everything BOINC needs for Android

./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --silent --arch arm
./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --silent --arch arm64
./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --silent --arch x86
./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --silent --arch x86_64

echo '===== BOINC for all platforms build done ====='
