#/bin/sh

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildApp#
#

# Script to compile various BOINC libraries for Android to be used
# by science applications

COMPILEBOINC="yes"
CONFIGURE="yes"
MAKECLEAN="yes"

export BOINC=".." #BOINC source code

export ANDROIDTC="$HOME/androidmips-tc"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/mipsel-linux-android"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib/libstdc++.a"

export PATH="$PATH:$TCBINARIES:$TCINCLUDES/bin"
export CC=mipsel-linux-android-gcc
export CXX=mipsel-linux-android-g++
export LD=mipsel-linux-android-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -DDECLARE_TIMEZONE -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -I$TCINCLUDES/include -funroll-loops -fexceptions -O3 -fomit-frame-pointer"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"
export PKG_CONFIG_SYSROOT_DIR=$TCSYSROOT

# Prepare android toolchain and environment
./build_androidtc_mips.sh

if [ -n "$COMPILEBOINC" ]; then
echo "==================building Libraries from $BOINC=========================="
cd $BOINC
if [ -n "$MAKECLEAN" ]; then
make clean
fi
if [ -n "$CONFIGURE" ]; then
./_autosetup
./configure --host=mipsel-linux --with-boinc-platform="mipsel-android-linux-gnu" --with-ssl=$TCINCLUDES --disable-server --disable-manager --disable-client --disable-shared --enable-static
fi
make
make stage

echo "=============================BOINC done============================="

fi
