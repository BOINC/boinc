#!/bin/sh
set -e

# Script to clear environment BOINC for Android

rm -rf ../3rdParty/buildCache/
rm -rf ../Makefile
rm -rf ../m4/Makefile

echo '===== Clear Environment done ====='
