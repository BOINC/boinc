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
export ANDROIDTC="${ANDROID_TC_ARM:-$ANDROID_TC/arm}"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/arm-linux-androideabi"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib/libstdc++.a"

export PATH="$TCBINARIES:$TCINCLUDES/bin:$PATH"
export CC=arm-linux-androideabi-clang
export CXX=arm-linux-androideabi-clang++
export LD=arm-linux-androideabi-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -D__ANDROID_API__=19"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -D__ANDROID_API__=19"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog -fPIE -pie -latomic -static-libstdc++ -march=armv7-a -Wl,--fix-cortex-a8"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"

# Prepare android toolchain and environment
./build_androidtc_arm.sh

run_command() {
    if [ "$2" = "silent" ]; then
        $1 1>/dev/null
    elif [ "$2" = "fullsilent" ]; then
        $1 &>/dev/null
    else
        $1
    fi
}

SILENT="verbose"
FULLSILENT="verbose"
if [ "$SILENT_MODE" = "yes" ]; then
SILENT="silent"
FULLSILENT="fullsilent"
fi

if [ "$COMPILECURL" = "yes" ]; then
echo "==================building curl from $CURL================================="
cd "$CURL"
if [ -n "$MAKECLEAN" ] && $(grep -q "^distclean:" "${CURL}/Makefile"); then
run_command 'make distclean' $FULLSILENT
fi
if [ -n "$CONFIGURE" ]; then
run_command './configure --host=arm-linux --prefix="$TCINCLUDES" --libdir="$TCINCLUDES/lib" --disable-shared --enable-static --with-random=/dev/urandom --without-zlib' $SILENT
fi
run_command 'make' $SILENT
run_command 'make install' $FULLSILENT
echo "========================curl done================================="
fi
