#/bin/sh

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile Libcurl for Android

COMPILECURL="yes"
CONFIGURE="yes"
MAKECLEAN="yes"

CURL="/home/boincadm/src/curl-7.28.1" #CURL sources, required by BOINC

export ANDROIDTC="$HOME/androidarm-tc"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/arm-linux-androideabi"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib/libstdc++.a"

export PATH="$PATH:$TCBINARIES:$TCINCLUDES/bin"
export CC=arm-linux-androideabi-gcc
export CXX=arm-linux-androideabi-g++
export LD=arm-linux-androideabi-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -funroll-loops -fexceptions -O3 -fomit-frame-pointer"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"

# Prepare android toolchain and environment
./build_androidtc_arm.sh

if [ -n "$COMPILECURL" ]; then
echo "==================building curl from $CURL================================="
cd $CURL
if [ -n "$MAKECLEAN" ]; then
make clean
fi
if [ -n "$CONFIGURE" ]; then
./configure --host=arm-linux --prefix=$TCINCLUDES --libdir="$TCINCLUDES/lib" --disable-shared --enable-static --with-random=/dev/urandom
fi
make
make install
echo "========================curl done================================="
fi
