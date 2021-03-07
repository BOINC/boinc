#!/bin/sh
set -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile OpenSSL for Android

COMPILEOPENSSL="${COMPILEOPENSSL:-yes}"
STDOUT_TARGET="${STDOUT_TARGET:-/dev/stdout}"
CONFIGURE="yes"
MAKECLEAN="yes"
VERBOSE="${VERBOSE:-no}"

OPENSSL="${OPENSSL_SRC:-$HOME/src/openssl-1.0.2p}" #openSSL sources, requiered by BOINC

export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROIDTC="${ANDROID_TC_ARMV6:-$ANDROID_TC/armv6}"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/arm-linux-androideabi"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib/libstdc++.a"

export PATH="$TCBINARIES:$TCINCLUDES/bin:$PATH"
export CC=arm-linux-androideabi-clang
export CXX=arm-linux-androideabi-clang++
export LD=arm-linux-androideabi-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE -march=armv6 -mfloat-abi=softfp -mfpu=vfp -D__ANDROID_API__=16 -DARMV6"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE -march=armv6 -mfloat-abi=softfp -mfpu=vfp -D__ANDROID_API__=16 -DARMV6"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog -fPIE -pie -latomic -static-libstdc++ -march=armv6"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"

# Prepare android toolchain and environment
./build_androidtc_armv6.sh

if [ "$COMPILEOPENSSL" = "yes" ]; then
    cd "$OPENSSL"
    echo "===== building openssl for armv6 from $PWD ====="
    if [ -n "$MAKECLEAN" ]; then
        if [ "$VERBOSE" = "no" ]; then
            make clean 1>$STDOUT_TARGET 2>&1
        else
            make clean SHELL="/bin/bash -x"
        fi
    fi
    if [ -n "$CONFIGURE" ]; then
        ./Configure linux-generic32 no-shared no-dso -DL_ENDIAN --openssldir="$TCINCLUDES" 1>$STDOUT_TARGET
        # override flags in Makefile
        sed -e "s/^CFLAG=.*$/`grep -e \^CFLAG= Makefile` \$(CFLAGS)/g" Makefile > Makefile.out
        mv Makefile.out Makefile
    fi
    if [ "$VERBOSE" = "no" ]; then
        make --silent 1>$STDOUT_TARGET
        make install_sw --silent 1>$STDOUT_TARGET
    else
        make SHELL="/bin/bash -x"
        make install_sw SHELL="/bin/bash -x"
    fi
    if  [ ! -z ${OPENSSL_FLAGFILE} ]; then
        touch "${OPENSSL_FLAGFILE}"
    fi	
    echo "===== openssl for armv6 build done ====="
fi
