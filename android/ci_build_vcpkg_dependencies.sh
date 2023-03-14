#!/bin/sh
set -e

#
# See: https://boinc.berkeley.edu/trac/wiki/AndroidBuildClient
#

# Script to compile BOINC dependencies (openssl and curl) for Android

# check working directory because the script needs to be called like: ./android/ci_build_vcpkg_dependencies.sh
if [ ! -d "android" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

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
    echo '===== BOINC dependencies for all platforms build start ====='
    android/build_component.sh --ci --component dependencies --with-vcpkg
    echo '===== BOINC dependencies for all platforms build done ====='
else
    echo '===== BOINC dependencies for ${arch} platform build start ====='
    android/build_component.sh --ci --component dependencies --arch $arch --with-vcpkg
    echo '===== BOINC dependencies for ${arch} platform build done ====='
fi
