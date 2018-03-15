#/bin/sh -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile Libcurl for Android

COMPILECURL="yes"
CONFIGURE="yes"
MAKECLEAN="yes"

CURL="${CURL_SRC:-$HOME/src/curl-7.48.0}" #CURL sources, required by BOINC

export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROIDTC="${ANDROID_TC_ARM:-$ANDROID_TC/arm}"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/arm-linux-androideabi"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib/libstdc++.a"

export PATH="$TCBINARIES:$TCINCLUDES/bin:$PATH"
export CC=arm-linux-androideabi-gcc
export CXX=arm-linux-androideabi-g++
export LD=arm-linux-androideabi-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -D__ANDROID_API__=16 -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog -fPIE -pie -march=armv7-a -Wl,--fix-cortex-a8"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"

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

if [ "x$cache_dir" != "x" ]; then
    PREFIX="$cache_dir/arm-linux-androideabi"
else
    PREFIX="$TCINCLUDES"
fi

FLAGFILE="${PREFIX}/_done"
if [ -e "${FLAGFILE}" ]; then
    echo "libcurl seems already to be present in ${PREFIX}"
    exit 0
fi

# Prepare android toolchain and environment
./build_androidtc_arm.sh
if [ $? -ne 0 ]; then exit 1; fi

if [ -n "$COMPILECURL" ]; then
echo "==================building curl from $CURL================================="
cd "$CURL"
if [ -n "$MAKECLEAN" ]; then
make clean
fi
if [ -n "$CONFIGURE" ]; then
./configure --host=arm-linux --prefix="$PREFIX" --libdir="$PREFIX/lib" --disable-shared --enable-static --with-random=/dev/urandom --without-zlib
if [ $? -ne 0 ]; then exit 1; fi
fi
make
if [ $? -ne 0 ]; then exit 1; fi
make install
if [ $? -ne 0 ]; then exit 1; fi
touch ${FLAGFILE}
echo "========================curl done================================="
fi
