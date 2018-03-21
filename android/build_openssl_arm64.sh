#/bin/sh -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to compile OpenSSL for Android

COMPILEOPENSSL="yes"
CONFIGURE="yes"
MAKECLEAN="yes"

OPENSSL="${OPENSSL_SRC:-$HOME/src/openssl-1.0.2g}" #openSSL sources, requiered by BOINC

export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROIDTC="${ANDROID_TC_ARM64:-$ANDROID_TC/arm64}"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/aarch64-linux-android"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib/libstdc++.a"

export PATH="$TCBINARIES:$TCINCLUDES/bin:$PATH"
export CC=aarch64-linux-android-gcc
export CXX=aarch64-linux-android-g++
export LD=aarch64-linux-android-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -D__ANDROID_API__=21 -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer -fPIE"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -funroll-loops -fexceptions -O3 -fomit-frame-pointer -fPIE"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog -fPIE -pie"
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
        PREFIX="$cache_dir/aarch64-linux-android"
    else
        echo "cache_dir must be an absolute path without ./ or ../ in it"
        exit 1
    fi
else
    PREFIX="$TCINCLUDES"
fi

FLAGFILE="${PREFIX}/_done"
if [ -e "${FLAGFILE}" ]; then
    echo "openssl seems already to be present in ${PREFIX}"
    exit 0
fi

# Prepare android toolchain and environment
./build_androidtc_arm64.sh --cache_dir "${PREFIX}"
if [ $? -ne 0 ]; then exit 1; fi

if [ -n "$COMPILEOPENSSL" ]; then
echo "================building openssl from $OPENSSL============================="
cd "$OPENSSL"
if [ -n "$MAKECLEAN" ]; then
make clean
fi
if [ -n "$CONFIGURE" ]; then
./Configure linux-generic32 no-shared no-dso -DL_ENDIAN --openssldir="$PREFIX/ssl"
if [ $? -ne 0 ]; then exit 1; fi
#override flags in Makefile
sed -e "s/^CFLAG=.*$/`grep -e \^CFLAG= Makefile` \$(CFLAGS)/g
s%^INSTALLTOP=.*%INSTALLTOP=$PREFIX%g" Makefile > Makefile.out
mv Makefile.out Makefile
fi
make
if [ $? -ne 0 ]; then exit 1; fi
make install_sw
if [ $? -ne 0 ]; then exit 1; fi
touch ${FLAGFILE}
echo "========================openssl DONE=================================="
fi
