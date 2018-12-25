#!/bin/sh
set -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile Libcurl for Android

COMPILECURL="${COMPILECURL:-yes}"
SILENT_MODE="${SILENT_MODE:-no}"
CONFIGURE="yes"
MAKECLEAN="yes"

CURL="${CURL_SRC:-$HOME/src/curl-7.61.0}" #CURL sources, required by BOINC

export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROIDTC="${ANDROID_TC_X86:-$ANDROID_TC/x86}"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/i686-linux-android"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib/libstdc++.a"

export PATH="$TCBINARIES:$TCINCLUDES/bin:$PATH"
export CC=i686-linux-android-clang
export CXX=i686-linux-android-clang++
export LD=i686-linux-android-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE -D__ANDROID_API__=19"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE -D__ANDROID_API__=19"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog -fPIE -pie -latomic -static-libstdc++"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"

# Prepare android toolchain and environment
./build_androidtc_x86.sh

if [ "$SILENT_MODE" = "yes" ]; then
    SILENT_MAKE="make 1>/dev/null"
    FULLSILENT_MAKE="make &>/dev/null"
    SILENT_CONFIGURE="./configure 1>/dev/null"
else
    SILENT_MAKE="make"
    FULLSILENT_MAKE="make"
    SILENT_CONFIGURE="./configure"
fi

if [ "$COMPILECURL" = "yes" ]; then
echo "==================building curl from $CURL================================="
cd "$CURL"
if [ -n "$MAKECLEAN" ] && $(grep -q "^distclean:" "${CURL}/Makefile"); then
$FULLSILENT_MAKE distclean
fi
if [ -n "$CONFIGURE" ]; then
$SILENT_CONFIGURE --host=i686-linux --prefix="$TCINCLUDES" --libdir="$TCINCLUDES/lib" --disable-shared --enable-static --with-random=/dev/urandom
fi
$SILENT_MAKE
$FULLSILENT_MAKE install
echo "========================curl done================================="
fi
