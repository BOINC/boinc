#/bin/sh

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile Libcurl for Android

COMPILECURL="yes"
CONFIGURE="yes"
MAKECLEAN="yes"

CURL="/home/boincadm/src/curl-7.48.0" #CURL sources, required by BOINC

export ANDROIDTC="$HOME/androidmips64-tc"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/mips64el-linux-android"
export TCSYSROOT="$ANDROIDTC/sysroot"

export PATH="$PATH:$TCBINARIES:$TCINCLUDES/bin"
export CC=mips64el-linux-android-gcc
export CXX=mips64el-linux-android-g++
export LD=mips64el-linux-android-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE"
export LDFLAGS="-L$TCSYSROOT/usr/lib64 -L$TCINCLUDES/lib64 -llog -fPIE -pie"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"

# Prepare android toolchain and environment
./build_androidtc_mips64.sh

if [ -n "$COMPILECURL" ]; then
echo "==================building curl from $CURL================================="
cd $CURL
if [ -n "$MAKECLEAN" ]; then
make distclean
fi
if [ -n "$CONFIGURE" ]; then
./configure --host=mip64sel-linux --prefix=$TCINCLUDES --libdir="$TCINCLUDES/lib" --disable-shared --enable-static --with-random=/dev/urandom
fi
make
make install
echo "========================curl done================================="
fi
