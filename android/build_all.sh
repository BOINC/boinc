#/bin/sh

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile everything BOINC needs for Android

./build_androidtc_arm.sh
./build_androidtc_x86.sh
./build_androidtc_mips.sh
./build_openssl_arm.sh
./build_openssl_x86.sh
./build_openssl_mips.sh
./build_curl_arm.sh
./build_curl_x86.sh
./build_curl_mips.sh
./build_boinc_arm.sh
./build_boinc_x86.sh
./build_boinc_mips.sh
