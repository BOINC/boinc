#!/bin/sh
set -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile OpenSSL for Android

COMPILEOPENSSL="yes"
CONFIGURE="yes"
MAKECLEAN="yes"

OPENSSL="${OPENSSL_SRC:-$HOME/src/openssl-1.0.2g}" #openSSL sources, requiered by BOINC

export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROIDTC="${ANDROID_TC_X86_64:-$ANDROID_TC/x86_64}"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/x86_64-linux-android"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib/libstdc++.a"

export PATH="$TCBINARIES:$TCINCLUDES/bin:$PATH"
export CC=x86_64-linux-android-gcc
export CXX=x86_64-linux-android-g++
export LD=x86_64-linux-android-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog -fPIE -pie"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"

# Prepare android toolchain and environment
./build_androidtc_x86_64.sh

if [ -n "$COMPILEOPENSSL" ]; then
echo "================building openssl from $OPENSSL============================="
cd "$OPENSSL"
if [ -n "$MAKECLEAN" ]; then
make clean
fi
if [ -n "$CONFIGURE" ]; then
./Configure linux-x86_64 no-shared no-dso -DL_ENDIAN --openssldir="$TCINCLUDES/ssl"
#override flags in Makefile
sed -e "s/^CFLAG=.*$/`grep -e \^CFLAG= Makefile` \$(CFLAGS)/g
s%^INSTALLTOP=.*%INSTALLTOP=$TCINCLUDES%g" Makefile > Makefile.out
mv Makefile.out Makefile
fi
make
make install_sw
echo "========================openssl DONE=================================="
fi
