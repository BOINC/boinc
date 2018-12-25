#!/bin/sh
set -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile OpenSSL for Android

COMPILEOPENSSL="${COMPILEOPENSSL:-yes}"
SILENT_MODE="${SILENT_MODE:-no}"
CONFIGURE="yes"
MAKECLEAN="yes"

OPENSSL="${OPENSSL_SRC:-$HOME/src/openssl-1.0.2p}" #openSSL sources, requiered by BOINC

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

if [ "$COMPILEOPENSSL" = "yes" ]; then
echo "================building openssl from $OPENSSL============================="
cd "$OPENSSL"
if [ -n "$MAKECLEAN" ]; then
run_command 'make clean' $FULLSILENT
fi
if [ -n "$CONFIGURE" ]; then
run_command './Configure linux-generic32 no-shared no-dso -DL_ENDIAN --openssldir="$TCINCLUDES/ssl"' $SILENT
#override flags in Makefile
sed -e "s/^CFLAG=.*$/`grep -e \^CFLAG= Makefile` \$(CFLAGS)/g
s%^INSTALLTOP=.*%INSTALLTOP=$TCINCLUDES%g" Makefile > Makefile.out
mv Makefile.out Makefile
fi
run_command 'make' $SILENT
run_command 'make install_sw' $FULLSILENT
echo "========================openssl DONE=================================="
fi
