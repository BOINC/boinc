#!/bin/sh
set -e

#
# See: https://github.com/BOINC/boinc/wiki/AndroidBuildClient
#

# Script to compile everything BOINC needs for Android

# check working directory because the script needs to be called like: ./android/ci_build_manager.sh
if [ ! -d "android" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

arch=""

while [ $# -gt 0 ]; do
    key="$1"
    case $key in
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

if [ "x$arch" = "x" ]; then
    echo '===== BOINC Client for all platforms build start ====='
    android/build_component.sh --ci --component client
    echo '===== BOINC Client for all platforms build done ====='
else
    echo '===== BOINC Client for '$arch' build start ====='
    android/build_component.sh --ci --component client --arch $arch
    echo '===== BOINC Client for '$arch' build done ====='
fi
