#!/bin/sh
set -e

# Script to compile everything BOINC needs for Linux flutter

# check working directory because the script needs to be called like: ./samples/flutter/ci_build_manager.sh
if [ ! -d "samples/flutter" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

echo '===== BOINC Flutuer Linux build start ====='
./_autosetup
linux/ci_configure_client.sh
make
cp "client/boinc" "samples/flutter/boinc/assets/linux/boinc"
cp "client/boinccmd" "samples/flutter/boinc/assets/linux/boinccmd"
cp "curl/ca-bundle.crt" "samples/flutter/boinc/assets/linux/ca-bundle.crt"
cat "samples/flutter/boinc/linux.yaml" >> "samples/flutter/boinc/pubspec.yaml"
echo '===== BOINC Flutuer Linux build done ====='
