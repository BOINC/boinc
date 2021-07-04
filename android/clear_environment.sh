#!/bin/sh
set -e

build_dir="3rdParty/android"
cache_dir="3rdParty/buildCache/android"

# Script to clear environment BOINC for Android

if [ ! -d android ]] ; then
    echo You run this script from diffrent folder: $PWD
    echo Please run it from boinc
    echo Type: \'android/clear_environment.sh\'
    exit 1
fi

if [ "android" = "$1" ]; then
    echo "android clean"
    rm -rf $cache_dir/android-tc
elif [ "vcpkg" = "$1" ]; then
    echo "vcpkg clean"
    rm -rf $build_dir/vcpkg
elif [ "ndk" = "$1" ]; then
    echo "ndk clean"
    rm -rf $build_dir/android-ndk-*
elif [ "vcpkg_cache" = "$1" ]; then
    echo "vcpkg_cache clean"
    rm -rf $cache_dir/vcpkgcache/
elif [ "cache" = "$1" ]; then
    echo "cache dir clean"
    rm -rf $cache_dir
elif [ "build" = "$1" ]; then
    echo "build dir clean"
    rm -rf $build_dir
elif [ "full" = "$1" ]; then
    echo "full clean:"
    echo "cache dir clean"
    echo "build dir clean"
    rm -rf $cache_dir
    rm -rf $build_dir
else
    echo "soft clean"
fi

rm -rf Makefile
rm -rf m4/Makefile

echo '===== Clear Environment done ====='
