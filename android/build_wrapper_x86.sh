#/bin/sh -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildApp#
#

# Script to compile various BOINC libraries for Android to be used
# by science applications

COMPILEBOINC="yes"
CONFIGURE="yes"
MAKECLEAN="yes"

export BOINC=".." #BOINC source code

export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROIDTC="${ANDROID_TC_X86:-$ANDROID_TC/x86}"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/i686-linux-android"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib/libstdc++.a"

export PATH="$TCBINARIES:$TCINCLUDES/bin:$PATH"
export CC=i686-linux-android-gcc
export CXX=i686-linux-android-g++
export LD=i686-linux-android-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -DDECLARE_TIMEZONE -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -I$TCINCLUDES/include -funroll-loops -fexceptions -O3 -fomit-frame-pointer"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"
export PKG_CONFIG_SYSROOT_DIR="$TCSYSROOT"
export PTHREAD=-L.

# Prepare android toolchain and environment
./build_androidtc_x86.sh
if [ $? -ne 0 ]; then exit 1; fi

if [ -n "$COMPILEBOINC" ]; then

echo "==================building Wrapper from $BOINC=========================="
cd "$BOINC"

if [ -n "$MAKECLEAN" ]; then
make clean
cd samples/wrapper
make clean
cd ../..
fi

if [ -n "$CONFIGURE" ]; then
./_autosetup
./configure --host=i686-linux --with-boinc-platform="x86-android-linux-gnu" --prefix="$TCINCLUDES" --libdir="$TCINCLUDES/lib" --with-ssl="$TCINCLUDES" --disable-server --disable-manager --disable-client --disable-shared --enable-static --enable-boinczip
if [ $? -ne 0 ]; then exit 1; fi
fi

make
if [ $? -ne 0 ]; then exit 1; fi

cd samples/wrapper
make
if [ $? -ne 0 ]; then exit 1; fi

echo "=============================Wrapper done============================="

fi
