#!/bin/sh
set -e

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2018 University of California
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

export OPENSSL_VERSION=1.0.2p
export CURL_VERSION=7.61.0
export NDK_VERSION=17c

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
         PREFIX="$build_dir"
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
fi

export COMPILEOPENSSL="no"
OPENSSL_FLAGFILE="$PREFIX/openssl-${OPENSSL_VERSION}_done"
if [ ! -e "${OPENSSL_FLAGFILE}" ]; then
    wget -O /tmp/openssl.tgz https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz
    tar xzf /tmp/openssl.tgz --directory=$BUILD_DIR
    export COMPILEOPENSSL="yes"
    touch "${OPENSSL_FLAGFILE}"
fi
export OPENSSL_SRC=$BUILD_DIR/openssl-${OPENSSL_VERSION}

export COMPILECURL="no"
CURL_FLAGFILE="$PREFIX/curl-${CURL_VERSION}_done"
if [ ! -e "${CURL_FLAGFILE}" ]; then
    wget -O /tmp/curl.tgz https://curl.haxx.se/download/curl-${CURL_VERSION}.tar.gz
    tar xzf /tmp/curl.tgz --directory=$BUILD_DIR
    export COMPILECURL="yes"
    touch "${CURL_FLAGFILE}"
fi
export CURL_SRC=$BUILD_DIR/curl-${CURL_VERSION}

export NDK_FLAGFILE="$PREFIX/NDK-${NDK_VERSION}-${arch}_done"
if [ ! -e "${NDK_FLAGFILE}" ]; then
    rm -rf "${PREFIX}/${arch}"
    wget -O /tmp/ndk.zip https://dl.google.com/android/repository/android-ndk-r${NDK_VERSION}-linux-x86_64.zip
    unzip -qq /tmp/ndk.zip -d $HOME
    touch "${NDK_FLAGFILE}"
fi
export NDK_ROOT=$HOME/android-ndk-r${NDK_VERSION}

export ANDROID_TC=$PREFIX

case "$arch" in
    "arm")
        ./build_androidtc_arm.sh
        ./build_openssl_arm.sh
        ./build_curl_arm.sh
        ./build_boinc_arm.sh
        exit 0
    ;;
    "arm64")
        ./build_androidtc_arm64.sh
        ./build_openssl_arm64.sh
        ./build_curl_arm64.sh
        ./build_boinc_arm64.sh
        exit 0
    ;;
    "x86")
        ./build_androidtc_x86.sh
        ./build_openssl_x86.sh
        ./build_curl_x86.sh
        ./build_boinc_x86.sh
        exit 0
    ;;
    "x86_64")
        ./build_androidtc_x86_64.sh
        ./build_openssl_x86_64.sh
        ./build_curl_x86_64.sh
        ./build_boinc_x86_64.sh
        exit 0
    ;;
esac

echo "unknown architecture: $arch"
exit 1