#!/bin/sh
set -e

#
# See: https://boinc.berkeley.edu/trac/wiki/AndroidBuildClient
#

# Script to compile everything BOINC needs for Android
export VERBOSE="no"

cd ../
BUILD_DIR="$PWD/3rdParty/buildCache"
cd android/

./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --arch arm
./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --arch arm64
./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --arch x86
./buildAndroidBOINC-CI.sh --cache_dir "$ANDROID_TC" --build_dir "$BUILD_DIR" --arch x86_64

ANDROID_TC="${ANDROID_TC:-../3rdParty/buildCache/android-tc/arm}"

if [ ! -d $ANDROID_TC ]; then
    echo You run this script from diffrent folder: $PWD
    echo Please run it from boinc/android
    exit 1
fi

list_libs="libcrypto.a libssl.a libcurl.a"

for i in $list_libs; do
    if [ $(readelf -A $(find $ANDROID_TC -name "$i") | grep -i neon | head -c1 | wc -c) -ne 0 ]; then
        echo "$i" is with neon optimization
        exit 1
    fi
done

echo '===== BOINC for all platforms build done ====='
