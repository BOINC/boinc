#!/bin/sh
set -e

# Script to clear environment BOINC for Android

if [ ! -d ../../boinc/android ]] ; then
    echo You run this script from diffrent folder: $PWD
    echo Please run it from boinc/android
    exit 1
fi

rm -rf ../3rdParty/buildCache/
rm -rf ../Makefile
rm -rf ../m4/Makefile

echo '===== Clear Environment done ====='
