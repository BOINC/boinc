#!/bin/sh
set -e

#
# See: https://boinc.berkeley.edu/trac/wiki/AndroidBuildClient
#

# Script to compile everything BOINC needs for Android

./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --silent --ci --arch arm
./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --silent --ci --arch arm64
./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --silent --ci --arch x86
./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --silent --ci --arch x86_64

echo ANDROID_TC=$ANDROID_TC

array_libs=("libcrypto.a" "libssl.a" "libcurl.a")

for i in "${array_libs[@]}"; do
    if [ $(readelf -A $(find $ANDROID_TC/arm  -name "$i") | grep -i neon) ]; then
        echo "$i" is with neon optimization
        exit 1
    fi
done

echo '===== BOINC for all platforms build done ====='
