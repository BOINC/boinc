#/bin/sh

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile everything BOINC needs for Android

./build_boinc_arm.sh
./build_boinc_x86.sh
./build_boinc_mips.sh
