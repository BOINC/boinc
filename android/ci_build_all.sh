#!/bin/sh
set -e

#
# See: https://boinc.berkeley.edu/trac/wiki/AndroidBuildClient
#

# Script to compile everything BOINC needs for Android

# check working directory because the script needs to be called like: ./android/ci_build_all.sh
if [ ! -d "android" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

cd android

echo '===== BOINC Client for all platforms build start ====='

./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --silent --ci --arch arm
./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --silent --ci --arch arm64
./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --silent --ci --arch x86
./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --silent --ci --arch x86_64

echo '===== BOINC Client for all platforms build done ====='

cd BOINC 

echo '===== BOINC Manager build start ====='

./gradlew clean assemble jacocoTestReportDebug

echo '===== BOINC Manager build done ====='
