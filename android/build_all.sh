#!/bin/sh
set -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile everything BOINC needs for Android

./build_androidtc_arm.sh
./build_androidtc_arm64.sh
./build_androidtc_x86.sh
./build_androidtc_x86_64.sh
./build_openssl_arm.sh
./build_openssl_arm64.sh
./build_openssl_x86.sh
./build_openssl_x86_64.sh
./build_curl_arm.sh
./build_curl_arm64.sh
./build_curl_x86.sh
./build_curl_x86_64.sh
./build_boinc_arm.sh
./build_boinc_arm64.sh
./build_boinc_x86.sh
./build_boinc_x86_64.sh
