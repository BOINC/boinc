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

android/ci_build_vcpkg_client.sh

cd android/BOINC

echo '===== BOINC Manager build start ====='

./gradlew clean assemble jacocoTestReportDebug --warning-mode all

echo '===== BOINC Manager build done ====='

cd ../../
