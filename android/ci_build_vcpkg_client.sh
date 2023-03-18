#!/bin/sh
set -e

#
# See: https://boinc.berkeley.edu/trac/wiki/AndroidBuildClient
#

# Script to compile everything BOINC needs for Android

# check working directory because the script needs to be called like: ./android/ci_build_manager.sh
if [ ! -d "android" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

echo '===== BOINC Client for all platforms build start ====='
android/build_component.sh --ci --component client
echo '===== BOINC Client for all platforms build done ====='
