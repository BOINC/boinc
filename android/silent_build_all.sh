#!/bin/sh
set -e

#
# See: https://boinc.berkeley.edu/trac/wiki/AndroidBuildClient
#

# Script to compile everything BOINC needs for Android
export VERBOSE="no"

./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --arch arm     --silent
./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --arch arm64   --silent
./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --arch x86     --silent
./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --arch x86_64  --silent

echo '===== BOINC for all platforms build done ====='
