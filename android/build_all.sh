#/bin/sh

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile everything BOINC needs for Android

./build_androidtc.sh
./build_openssl.sh
./build_curl.sh
./build_boinc.sh
