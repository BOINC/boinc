#/bin/sh

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile BOINC for Android

COMPILEBOINC="yes"
CONFIGURE="yes"
MAKECLEAN="yes"

export BOINC=".." #BOINC source code

export ANDROIDTC="$HOME/androidarm64-tc"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/aarch64-linux-android"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib/libstdc++.a"

export PATH="$PATH:$TCBINARIES:$TCINCLUDES/bin"
export CC=aarch64-linux-android-gcc
export CXX=aarch64-linux-android-g++
export LD=aarch64-linux-android-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -DANDROID_64 -DDECLARE_TIMEZONE -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -DANDROID_64 -Wall -I$TCINCLUDES/include -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog -fPIE -pie"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"
export PKG_CONFIG_SYSROOT_DIR=$TCSYSROOT

# Prepare android toolchain and environment
./build_androidtc_arm64.sh

if [ -n "$COMPILEBOINC" ]; then
echo "==================building BOINC from $BOINC=========================="
cd $BOINC
if [ -n "$MAKECLEAN" ]; then
make distclean
fi
if [ -n "$CONFIGURE" ]; then
./_autosetup
./configure --host=aarch64-linux --with-boinc-platform="aarch64-android-linux-gnu" --with-boinc-alt-platform="arm-android-linux-gnu" --with-ssl=$TCINCLUDES --disable-server --disable-manager --disable-shared --enable-static
sed -e "s%^CLIENTLIBS *= *.*$%CLIENTLIBS = -lm $STDCPPTC%g" client/Makefile > client/Makefile.out
mv client/Makefile.out client/Makefile
fi
make
make stage

echo "Stripping Binaries"
cd stage/usr/local/bin
aarch64-linux-android-strip *
cd ../../../../

echo "Copy Assets"
cd android
mkdir "BOINC/app/src/main/assets"
cp "$BOINC/stage/usr/local/bin/boinc" "BOINC/app/src/main/assets/arm64-v8a/boinc"
cp "$BOINC/stage/usr/local/bin/boinccmd" "BOINC/app/src/main/assets/arm64-v8a/boinccmd"
cp "$BOINC/win_build/installerv2/redist/all_projects_list.xml" "BOINC/app/src/main/assets/all_projects_list.xml"
cp "$BOINC/curl/ca-bundle.crt" "BOINC/app/src/main/assets/ca-bundle.crt"

echo "=============================BOINC done============================="

fi
