#!/bin/sh
set -e

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2020 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
#

export OPENSSL_VERSION=1.0.2s
export CURL_VERSION=7.62.0
export NDK_VERSION=21d

export ANDROID_HOME=$HOME/Android/Sdk
export NDK_ROOT=$HOME/Android/Ndk
export ANDROID_TC=$HOME/Android/Toolchains


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

doclean=""
cache_dir=""
arch=""
silent=""
verbose="${VERBOSE:-no}"
ci=""

while [ $# -gt 0 ]; do
    key="$1"
    case $key in
        --cache_dir)
        cache_dir="$2"
        shift
        ;;
        --build_dir)
        build_dir="$2"
        shift
        ;;
        --clean)
        doclean="yes"
        ;;
        --arch)
        arch="$2"
        shift
        ;;
        --silent)
        silent="yes"
        ;;
        --verbose)
        verbose="yes"
        ;;
        --ci)
        ci="yes"
        ;;
        *)
        echo "unrecognized option $key"
        ;;
    esac
    shift # past argument or value
done

if [ "x$cache_dir" != "x" ]; then
    if isPathCanonical "$cache_dir" && [ "$cache_dir" != "/" ]; then
         PREFIX="$cache_dir"
     else
         echo "cache_dir must be an absolute path without ./ or ../ in it"
         exit 1
     fi
else
    cd ../
    PREFIX="$(pwd)/3rdParty/buildCache/android-tc"
    cd android/
fi

if [ "x$build_dir" != "x" ]; then
    if isPathCanonical "$build_dir" && [ "$build_dir" != "/" ]; then
         BUILD_DIR="$build_dir"
     else
         echo "build_dir must be an absolute path without ./ or ../ in it"
         exit 1
     fi
else
    cd ../
    BUILD_DIR="$(pwd)/3rdParty/android"
    cd android/
fi

mkdir -p "${PREFIX}"
mkdir -p "${BUILD_DIR}"

if [ "${doclean}" = "yes" ]; then
    echo "cleaning cache"
    rm -rf "${PREFIX}"
    mkdir -p "${PREFIX}"
    echo "cleaning build dir"
    rm -rf "${BUILD_DIR}"
    mkdir -p "${BUILD_DIR}"
    echo "cleaning downloaded cache files"
    rm -f /tmp/ndk_${NDK_VERSION}.zip
    rm -f /tmp/openssl_${OPENSSL_VERSION}.tgz
    rm -f /tmp/curl_${CURL_VERSION}.tgz
fi

if [ "${silent}" = "yes" ]; then
    export STDOUT_TARGET="/dev/null"
fi

export COMPILEOPENSSL="no"
export COMPILECURL="no"
export NDK_FLAGFILE="$PREFIX/NDK-${NDK_VERSION}-${arch}_done"
export CURL_FLAGFILE="$PREFIX/curl-${CURL_VERSION}-${arch}_done"
export OPENSSL_FLAGFILE="$PREFIX/openssl-${OPENSSL_VERSION}-${arch}_done"
export CREATED_NDK_FOLDER=${CREATED_NDK_FOLDER:-"no"}

createNDKFolder()
{
    if [ $CREATED_NDK_FOLDER = "no" ]; then
        rm -rf "$BUILD_DIR/android-ndk-r${NDK_VERSION}"
        wget -c --no-verbose -O /tmp/ndk_${NDK_VERSION}.zip https://dl.google.com/android/repository/android-ndk-r${NDK_VERSION}-linux-x86_64.zip
        unzip -qq /tmp/ndk_${NDK_VERSION}.zip -d $BUILD_DIR
        export CREATED_NDK_FOLDER="yes"
        touch "${NDK_FLAGFILE}"
    fi
}

if [ ci = "yes" ]; then
    if [ ! -e "${NDK_FLAGFILE}" ]; then
        rm -rf $OPENSSL_FLAGFILE
        rm -rf $CURL_FLAGFILE
        export CREATED_NDK_FOLDER="no"
    fi
    createNDKFolder
else
    if [ ! -e "${NDK_FLAGFILE}" ]; then
        rm -rf $OPENSSL_FLAGFILE
        rm -rf $CURL_FLAGFILE
        export CREATED_NDK_FOLDER="no"
        createNDKFolder
    fi
fi

export NDK_ROOT=$BUILD_DIR/android-ndk-r${NDK_VERSION}

if [ ! -e "${OPENSSL_FLAGFILE}" ]; then
    rm -rf "$BUILD_DIR/openssl-${OPENSSL_VERSION}"
    wget -c --no-verbose -O /tmp/openssl_${OPENSSL_VERSION}.tgz https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz
    tar xzf /tmp/openssl_${OPENSSL_VERSION}.tgz --directory=$BUILD_DIR
    export COMPILEOPENSSL="yes"
fi
export OPENSSL_SRC=$BUILD_DIR/openssl-${OPENSSL_VERSION}

if [ ! -e "${CURL_FLAGFILE}" ]; then
    rm -rf "$BUILD_DIR/curl-${CURL_VERSION}"
    wget -c --no-verbose -O /tmp/curl_${CURL_VERSION}.tgz https://curl.haxx.se/download/curl-${CURL_VERSION}.tar.gz
    tar xzf /tmp/curl_${CURL_VERSION}.tgz --directory=$BUILD_DIR
    export COMPILECURL="yes"
fi
export CURL_SRC=$BUILD_DIR/curl-${CURL_VERSION}

export ANDROID_TC=$PREFIX

export VERBOSE=$verbose

case "$arch" in
    "arm")
        ./build_openssl_arm.sh
        ./build_curl_arm.sh
        ./build_boinc_arm.sh
        exit 0
    ;;
    "arm64")
        ./build_openssl_arm64.sh
        ./build_curl_arm64.sh
        ./build_boinc_arm64.sh
        exit 0
    ;;
    "x86")
        ./build_openssl_x86.sh
        ./build_curl_x86.sh
        ./build_boinc_x86.sh
        exit 0
    ;;
    "x86_64")
        ./build_openssl_x86_64.sh
        ./build_curl_x86_64.sh
        ./build_boinc_x86_64.sh
        exit 0
    ;;
esac

echo "unknown architecture: $arch"
exit 1
