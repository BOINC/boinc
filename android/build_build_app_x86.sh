#/bin/sh -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildApp
#

# Script to compile a generic application on Android

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

export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
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

make clean

if [ -e "./confgiure" ]; then
./configure --host=i686-linux --prefix="$PREFIX" --libdir="$PREFIX/lib" --with-ssl="$PREFIX" --with-libcurl="$PREFIX" --disable-shared --enable-static
if [ $? -ne 0 ]; then exit 1; fi
fi

make
if [ $? -ne 0 ]; then exit 1; fi
