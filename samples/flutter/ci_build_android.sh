#!/bin/sh
set -e

# Script to compile everything BOINC needs for Android flutter

# check working directory because the script needs to be called like: ./samples/flutter/ci_build_manager.sh
if [ ! -d "samples/flutter" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

echo '===== BOINC Flutuer Android build start ====='
android/ci_build_vcpkg_client.sh
cp "android/BOINC/app/src/main/assets/arm64-v8a/boinc" "samples/flutter/boinc/assets/arm64-v8a/boinc"
cp "android/BOINC/app/src/main/assets/armeabi/boinc" "samples/flutter/boinc/assets/armeabi/boinc"
cp "android/BOINC/app/src/main/assets/armeabi-v7a/boinc" "samples/flutter/boinc/assets/armeabi-v7a/boinc"
cp "android/BOINC/app/src/main/assets/x86/boinc" "samples/flutter/boinc/assets/x86/boinc"
cp "android/BOINC/app/src/main/assets/x86_64/boinc" "samples/flutter/boinc/assets/x86_64/boinc"
cp "win_build/installerv2/redist/all_projects_list.xml" "samples/flutter/boinc/assets/all_projects_list.xml"
cp "curl/ca-bundle.crt" "samples/flutter/boinc/assets/ca-bundle.crt"
cp "android/BOINC/app/src/main/assets/cc_config.xml" "samples/flutter/boinc/assets/cc_config.xml"
cp "android/BOINC/app/src/main/assets/nomedia" "samples/flutter/boinc/assets/nomedia"
cat "samples/flutter/boinc/android.yaml" >> "samples/flutter/boinc/pubspec.yaml"
android/clear_environment.sh full
echo '===== BOINC Flutuer Android build done ====='
