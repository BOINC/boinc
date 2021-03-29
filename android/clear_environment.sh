#!/bin/sh
set -e

# Script to clear environment BOINC for Android

if [ ! -d android ]] ; then
    echo You run this script from diffrent folder: $PWD
    echo Please run it from boinc
    echo Type: \'android/clear_environment.sh\'
    exit 1
fi

if [ "full" = "$1" ]; then
    echo "full clean"
    rm -rf 3rdParty/buildCache/android-tc
else
    echo "soft clean"
fi
    rm -rf Makefile
    rm -rf m4/Makefile

echo '===== Clear Environment done ====='
