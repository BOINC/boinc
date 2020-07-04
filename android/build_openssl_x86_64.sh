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

export NDK_ROOT=${NDK_ROOT:-$HOME/Android/Ndk}
export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROIDTC="${ANDROID_TC_X86_64:-$ANDROID_TC/x86_64}"
export TOOLCHAINROOT="$NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/"
export TCBINARIES="$TOOLCHAINROOT/bin"
export TCINCLUDES="$ANDROIDTC/x86_64-linux-android"
export TCSYSROOT="$TOOLCHAINROOT/sysroot"

export PATH="$TCBINARIES:$TCINCLUDES/bin:$PATH"
export CC=x86_64-linux-android21-clang
export CXX=x86_64-linux-android21-clang++
export LD=x86_64-linux-android-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE -D__ANDROID_API__=21"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -I$TCINCLUDES/include -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE -D__ANDROID_API__=21"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog -fPIE -pie -latomic -static-libstdc++"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"

if [ "$COMPILEOPENSSL" = "yes" ]; then
    cd "$OPENSSL"
    echo "===== building openssl for x86-64 from $PWD ====="
    if [ -n "$MAKECLEAN" ]; then
        if [ "$VERBOSE" = "no" ]; then
            make clean 1>$STDOUT_TARGET 2>&1
        else
            make clean SHELL="/bin/bash -x"
        fi
    fi
    if [ -n "$CONFIGURE" ]; then
        ./Configure linux-x86_64 no-shared no-dso -DL_ENDIAN --openssldir="$TCINCLUDES" 1>$STDOUT_TARGET
        #override flags in Makefile
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
    echo "===== openssl for x86-64 build done ====="
fi
