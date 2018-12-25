#!/bin/sh
set -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile BOINC for Android

SILENT_MODE="${SILENT_MODE:-no}"
COMPILEBOINC="yes"
CONFIGURE="yes"
MAKECLEAN="yes"

export BOINC=".." #BOINC source code

export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROIDTC="${ANDROID_TC_X86-$ANDROID_TC/x86}"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/i686-linux-android"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib/libstdc++.a"

export PATH="$TCBINARIES:$TCINCLUDES/bin:$PATH"
export CC=i686-linux-android-clang
export CXX=i686-linux-android-clang++
export LD=i686-linux-android-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -DDECLARE_TIMEZONE -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE -D__ANDROID_API__=19"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -I$TCINCLUDES/include -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE -D__ANDROID_API__=19"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog -fPIE -pie -latomic -static-libstdc++"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"
export PKG_CONFIG_SYSROOT_DIR="$TCSYSROOT"

# Prepare android toolchain and environment
./build_androidtc_x86.sh

run_command() {
    if [ "$2" = "silent" ]; then
        $1 --silent
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

if [ -n "$COMPILEBOINC" ]; then
echo "==================building BOINC from $BOINC=========================="
cd "$BOINC"
if [ -n "$MAKECLEAN" ] && [ -f "Makefile" ]; then
run_command 'make distclean' $FULLSILENT
fi
if [ -n "$CONFIGURE" ]; then
./_autosetup
./configure --host=i686-linux --with-boinc-platform="x86-android-linux-gnu" --with-ssl="$TCINCLUDES" --disable-server --disable-manager --disable-shared --enable-static --disable-largefile
sed -e "s%^CLIENTLIBS *= *.*$%CLIENTLIBS = -lm $STDCPPTC%g" client/Makefile > client/Makefile.out
mv client/Makefile.out client/Makefile
fi
run_command 'make' $SILENT
run_command 'make stage' $SILENT

echo "Stripping Binaries"
cd stage/usr/local/bin
i686-linux-android-strip *
cd ../../../../

echo "Copy Assets"
cd android
mkdir -p "BOINC/app/src/main/assets"
cp "$BOINC/stage/usr/local/bin/boinc" "BOINC/app/src/main/assets/x86/boinc"
cp "$BOINC/stage/usr/local/bin/boinccmd" "BOINC/app/src/main/assets/x86/boinccmd"
cp "$BOINC/win_build/installerv2/redist/all_projects_list.xml" "BOINC/app/src/main/assets/all_projects_list.xml"
cp "$BOINC/curl/ca-bundle.crt" "BOINC/app/src/main/assets/ca-bundle.crt"

echo "=============================BOINC done============================="

fi
