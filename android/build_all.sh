#!/bin/sh
set -e

#
# See: https://boinc.berkeley.edu/trac/wiki/AndroidBuildClient
#

# Script to compile everything BOINC needs for Android
export VERBOSE=false
cd ../
BUILD_DIR="$PWD/3rdParty/buildCache"
cd android/

./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --arch arm
./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --arch arm64
./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --arch x86
./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --arch x86_64

echo '===== BOINC for all platforms build done ====='
