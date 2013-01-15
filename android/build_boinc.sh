#/bin/sh
#script to compile BOINC for Android

#++++++++++++++++++++++++CONFIGURATION++++++++++++++++++++++++++++

#===locations
#Android toolchain
export ANDROIDTC="/tmp/tc"

#sources
export BOINC=".." #BOINC source code
export OPENSSL_DIR=$BOINC/../boinc_depends_android_eclipse/openssl
export CURL_DIR=$BOINC/../boinc_depends_android_eclipse/curl

#===script behavior
export PKG_CONFIG_DEBUG_SPEW=1

CONFIGURE="yes"
MAKECLEAN="yes"

COMPILEBOINC="yes"
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

export TCBINARIES="$ANDROIDTC/bin" #cross compiler location
export TCINCLUDES="$ANDROIDTC/arm-linux-androideabi" #libraries to include in build
export TCSYSROOT="$ANDROIDTC/sysroot" #SYSROOT of Android device
export STDCPPTC="$TCINCLUDES/lib/libstdc++.a" #stdc++ library

export PATH="$PATH:$TCBINARIES:$TCINCLUDES/bin" #add location of compiler binaries to PATH
export CC=arm-linux-androideabi-gcc #C compiler
export CXX=arm-linux-androideabi-g++ #C++ compiler
export LD=arm-linux-androideabi-ld #LD tool
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -DDECLARE_TIMEZONE -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall  -I$TCINCLUDES/include -funroll-loops -fexceptions -O3 -fomit-frame-pointer"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog" #log is Logcat
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"
export PKG_CONFIG_SYSROOT_DIR=$TCSYSROOT
export PKG_CONFIG_PATH=$CURL_DIR/lib/pkgconfig:$OPENSSL_DIR/lib/pkgconfig


if [ -n "$COMPILEBOINC" ]; then
echo "==================building BOINC from $BOINC=========================="
cd $BOINC
if [ -n "$MAKECLEAN" ]; then
make clean
fi
if [ -n "$CONFIGURE" ]; then
./_autosetup
./configure --host=arm-linux --with-boinc-platform="arm-android-linux-gnu" --with-ssl=$TCINCLUDES --disable-server --disable-manager --disable-shared --enable-static
sed -e "s%^CLIENTLIBS *= *.*$%CLIENTLIBS = -lm $STDCPPTC%g" client/Makefile > client/Makefile.out
mv client/Makefile.out client/Makefile
fi
make
make stage

echo "Stripping Binaries"
cd stage/usr/local/bin
arm-linux-androideabi-strip *
cd ../../../..

echo "=============================BOINC done============================="

fi
