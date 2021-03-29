#!/bin/sh
set -e

if [ ! -d "android" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

export VERBOSE="no"

echo '===== BOINC apps for all platforms build start ====='
android/build_component.sh --component apps
echo '===== BOINC apps for all platforms build done ====='
