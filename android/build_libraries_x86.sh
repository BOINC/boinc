#/bin/sh -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildApp#
#

# Script to compile various BOINC libraries for Android to be used
# by science applications

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
        PREFIX="$cache_dir/i686-linux-android"
    else
        echo "cache_dir must be an absolute path without ./ or ../ in it"
        exit 1
    fi
else
    PREFIX="$TCINCLUDES"
fi

if [ "x$TRAVIS_BUILD_DIR" != "x" ]; then
    export BUILD_DIR="$TRAVIS_BUILD_DIR"
else
    export BUILD_DIR="$HOME"
fi

export BOINC=".." #BOINC source code

export ANDROID_TC="${ANDROID_TC:-$BUILD_DIR/android-tc}"
export ANDROIDTC="${ANDROID_TC_X86:-$ANDROID_TC/x86}"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/i686-linux-android"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib/libstdc++.a"

export PATH="$PREFIX/bin:$PREFIX:$TCBINARIES:$TCINCLUDES:$TCINCLUDES/bin:$PATH"
export CC=i686-linux-android-gcc
export CXX=i686-linux-android-g++
export LD=i686-linux-android-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -D__ANDROID_API__=16 -DDECLARE_TIMEZONE -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -D__ANDROID_API__=16 -Wall -isystem $ANDROIDTC/include/c++/4.9.x -idirafter $ANDROIDTC/lib/gcc/i686-linux-android/4.9.x/include -I$TCINCLUDES/include -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog -lc++_shared -fPIE -pie"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"
export PKG_CONFIG_SYSROOT_DIR="$TCSYSROOT"

# Prepare android toolchain and environment
./build_androidtc_x86.sh --cache_dir "${PREFIX}"
if [ $? -ne 0 ]; then exit 1; fi

if [ -n "$COMPILEBOINC" ]; then

echo "==================building Libraries from $BOINC=========================="
cd "$BOINC"
if [ -n "$MAKECLEAN" ]; then
make clean
fi
if [ -n "$CONFIGURE" ]; then
./_autosetup
./configure --host=i686-linux --with-boinc-platform="x86-android-linux-gnu" --prefix="$PREFIX" --libdir="$PREFIX/lib" --with-ssl="$PREFIX" --with-libcurl="$PREFIX" --disable-server --disable-manager --disable-client --disable-shared --enable-static --enable-boinczip
if [ $? -ne 0 ]; then exit 1; fi
fi
make
if [ $? -ne 0 ]; then exit 1; fi
make stage
if [ $? -ne 0 ]; then exit 1; fi
make install
if [ $? -ne 0 ]; then exit 1; fi

echo "=============================BOINC done============================="

fi
