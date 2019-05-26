#!/bin/sh
set -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile everything BOINC needs for Android

./buildAndroidBOINC-CI.sh --cache_dir $ANDROID_TC --build_dir $HOME/3rdParty --arch arm
./buildAndroidBOINC-CI.sh --cache_dir $ANDROID_TC --build_dir $HOME/3rdParty --arch arm64
./buildAndroidBOINC-CI.sh --cache_dir $ANDROID_TC --build_dir $HOME/3rdParty --arch x86
./buildAndroidBOINC-CI.sh --cache_dir $ANDROID_TC --build_dir $HOME/3rdParty --arch x86_64
