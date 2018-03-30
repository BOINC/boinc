#!/bin/sh
set -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile BOINC for Android

COMPILEBOINC="yes"
CONFIGURE="yes"
MAKECLEAN="yes"

cache_dir=""
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        --cache_dir)
        cache_dir="$2"
        shift
        ;;
        *)
        echo "unrecognized option $key"
        ;;
    esac
    shift # past argument or value
done

# checks if a given path is canonical (absolute and does not contain relative links)
# from http://unix.stackexchange.com/a/256437
isPathCanonical() {
  case "x$1" in
    (x*/..|x*/../*|x../*|x*/.|x*/./*|x./*)
        rc=1
        ;;
    (x/*)
        rc=0
        ;;
    (*)
        rc=1
        ;;
  esac
  return $rc
}

if [ "x$cache_dir" != "x" ]; then
    if isPathCanonical "$cache_dir" && [ "$cache_dir" != "/" ]; then
        PREFIX="$cache_dir/x86_64-linux-android"
    else
        echo "cache_dir must be an absolute path without ./ or ../ in it"
        exit 1
    fi
else
    PREFIX="$TCINCLUDES"
fi

export BOINC=".." #BOINC source code

export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROIDTC="${ANDROID_TC_X86_64-$ANDROID_TC/x86_64}"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/x86_64-linux-android"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib64/libstdc++.a"

export PATH="$PREFIX/bin:$PREFIX:$TCBINARIES:$TCINCLUDES:$TCINCLUDES/bin:$PATH"
export CC=x86_64-linux-android-gcc
export CXX=x86_64-linux-android-g++
export LD=x86_64-linux-android-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -DANDROID_64 -D__ANDROID_API__=21 -DDECLARE_TIMEZONE -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -DANDROID_64 -D__ANDROID_API__=21 -Wall -isystem $ANDROIDTC/include/c++/4.9.x -idirafter $ANDROIDTC/lib/gcc/x86_64-linux-android/4.9.x/include -I$TCINCLUDES/include -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE"
export LDFLAGS="-L$TCSYSROOT/usr/lib64 -L$TCINCLUDES/lib64 -llog -lc++_shared -fPIE -pie"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"
export PKG_CONFIG_SYSROOT_DIR="$TCSYSROOT"

# Prepare android toolchain and environment
./build_androidtc_x86_64.sh --cache_dir "${PREFIX}"

if [ -n "$COMPILEBOINC" ]; then
echo "==================building BOINC from $BOINC=========================="
cd "$BOINC"
if [ -n "$MAKECLEAN" ]; then
make distclean
fi
if [ -n "$CONFIGURE" ]; then
./_autosetup
./configure --host=x86_64-linux --with-boinc-platform="x86_64-android-linux-gnu" --with-boinc-alt-platform="x86-android-linux-gnu" --with-ssl="$PREFIX" --with-libcurl="$PREFIX" --disable-server --disable-manager --disable-shared --enable-static
sed -e "s%^CLIENTLIBS *= *.*$%CLIENTLIBS = -lm $STDCPPTC%g" client/Makefile > client/Makefile.out
mv client/Makefile.out client/Makefile
fi
make
make stage

echo "Stripping Binaries"
cd stage/usr/local/bin
x86_64-linux-android-strip *
cd ../../../../

echo "Copy Assets"
cd android
mkdir -p "BOINC/app/src/main/assets"
cp "$BOINC/stage/usr/local/bin/boinc" "BOINC/app/src/main/assets/x86_64/boinc"
cp "$BOINC/stage/usr/local/bin/boinccmd" "BOINC/app/src/main/assets/x86_64/boinccmd"
cp "$BOINC/win_build/installerv2/redist/all_projects_list.xml" "BOINC/app/src/main/assets/all_projects_list.xml"
cp "$BOINC/curl/ca-bundle.crt" "BOINC/app/src/main/assets/ca-bundle.crt"

echo "=============================BOINC done============================="

fi
