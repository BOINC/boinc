#!/bin/sh
set -e

#
# See: https://github.com/BOINC/boinc/wiki/AndroidBuildClient
#

# Script to compile BOINC apps for Android

# check working directory because the script needs to be called like: ./android/ci_build_apps.sh
if [ ! -d "android" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

echo '===== BOINC apps for all platforms build start ====='
android/build_component.sh --ci --component apps
echo '===== BOINC apps for all platforms build done ====='

