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

skip_client_build=""
tasks=""
while [ $# -gt 0 ]; do
    key="$1"
    case $key in
        --skip-client-build)
        skip_client_build="yes"
        ;;
        --tasks)
        tasks="$2"
        shift
        ;;
        *)
        echo "unrecognized option $key"
        ;;
    esac
    shift
done

if [ "x$skip_client_build" = "x" ]; then
    android/ci_build_vcpkg_client.sh
fi

cd android/BOINC

echo '===== BOINC Manager build start ====='

if [ "x$tasks" = "x" ]; then
    tasks="clean assemble jacocoTestReportDebug"
fi
./gradlew $tasks --warning-mode all

echo '===== BOINC Manager build done ====='

cd ../../
