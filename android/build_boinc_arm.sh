#/bin/sh

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile BOINC for Android

COMPILEBOINC="yes"
CONFIGURE="yes"
MAKECLEAN="yes"

export BOINC=".." #BOINC source code

export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROIDTC="${ANDROID_TC_ARM:-$ANDROID_TC/arm}"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/arm-linux-androideabi"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib/libstdc++.a"

echo "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT"
echo $TCINCLUDES

export PATH="$PATH:$TCBINARIES:$TCINCLUDES/bin"
export CC=arm-linux-androideabi-gcc
export CXX=arm-linux-androideabi-g++
export LD=arm-linux-androideabi-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -D__ANDROID_API__=16 -DDECLARE_TIMEZONE -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE -march=armv7-a"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -D__ANDROID_API__=16 -Wall -nostdinc -idirafter $ANDROIDTC/arm/lib/gcc/arm-linux-androideabi/4.9.x/include -I$TCINCLUDES/include -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE -march=armv7-a"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog -lc++_shared -fPIE -pie -march=armv7-a -Wl,--fix-cortex-a8"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"
export PKG_CONFIG_SYSROOT_DIR=$TCSYSROOT

# Prepare android toolchain and environment
./build_androidtc_arm.sh
if [ $? -ne 0 ]; then exit 1; fi

if [ -n "$COMPILEBOINC" ]; then
echo "==================building BOINC from $BOINC=========================="
cd $BOINC
if [ -n "$MAKECLEAN" ]; then
make distclean
fi
if [ -n "$CONFIGURE" ]; then
./_autosetup
./configure --host=arm-linux --with-boinc-platform="arm-android-linux-gnu" --with-ssl=$TCINCLUDES --with-libcurl=$TCINCLUDES --disable-server --disable-manager --disable-shared --enable-static
if [ $? -ne 0 ]; then exit 1; fi
sed -e "s%^CLIENTLIBS *= *.*$%CLIENTLIBS = -lm $STDCPPTC%g" client/Makefile > client/Makefile.out
mv client/Makefile.out client/Makefile
fi
make
if [ $? -ne 0 ]; then exit 1; fi
make stage
if [ $? -ne 0 ]; then exit 1; fi

echo "Stripping Binaries"
cd stage/usr/local/bin
arm-linux-androideabi-strip *
cd ../../../../

echo "Copy Assets"
cd android
mkdir -p "BOINC/app/src/main/assets"
cp "$BOINC/stage/usr/local/bin/boinc" "BOINC/app/src/main/assets/armeabi-v7a/boinc"
cp "$BOINC/stage/usr/local/bin/boinccmd" "BOINC/app/src/main/assets/armeabi-v7a/boinccmd"
cp "$BOINC/win_build/installerv2/redist/all_projects_list.xml" "BOINC/app/src/main/assets/all_projects_list.xml"
cp "$BOINC/curl/ca-bundle.crt" "BOINC/app/src/main/assets/ca-bundle.crt"

echo "=============================BOINC done============================="

fi
