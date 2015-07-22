#/bin/sh

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildApp
#

# Script to compile a generic application on Android

export ANDROIDTC="$HOME/androidx86-tc"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/i686-linux-android"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib/libstdc++.a"

export PATH="$PATH:$TCBINARIES:$TCINCLUDES/bin"
export CC=i686-linux-android-gcc
export CXX=i686-linux-android-g++
export LD=i686-linux-android-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -O3 -fomit-frame-pointer -fPIE"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog -fPIE -pie"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"

make clean

if [ -e "./confgiure" ]; then
./configure --host=i686-linux --prefix=$TCINCLUDES --libdir="$TCINCLUDES/lib" --disable-shared --enable-static
fi

make
