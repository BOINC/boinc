#!/bin/sh
set -e

# Script to compile everything BOINC needs for macOS flutter

# check working directory because the script needs to be called like: ./samples/flutter/ci_build_manager.sh
if [ ! -d "samples/flutter" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

echo '===== BOINC Flutuer macOS build start ====='
3rdParty/buildMacDependencies.sh -q
mac_build/buildMacBOINC-CI.sh --no_shared_headers

cp "mac_build/build/Deployment/boinc" "samples/flutter/boinc/assets/macos/boinc"
cp "curl/ca-bundle.crt" "samples/flutter/boinc/assets/macos/ca-bundle.crt"
cat "samples/flutter/boinc/macos.yaml" >> "samples/flutter/boinc/pubspec.yaml"
echo '===== BOINC Flutuer macOS build done ====='
