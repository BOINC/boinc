#!/bin/bash

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

if [ "x$TRAVIS_BUILD_DIR" != "x" ]; then
    export BUILD_DIR="$TRAVIS_BUILD_DIR"
else
    export BUILD_DIR="$HOME"
fi

umake android android-studio --accept-license $BUILD_DIR/Android/Android-Studio
export ANDROID_HOME=$BUILD_DIR/Android/Sdk
umake android android-sdk --accept-license $BUILD_DIR/Android/Sdk
export NDK_ROOT=$BUILD_DIR/Android/Ndk
umake android android-ndk --accept-license $BUILD_DIR/Android/Ndk
yes | $BUILD_DIR/Android/Sdk/tools/bin/sdkmanager --update >> /dev/null
yes | $BUILD_DIR/Android/Sdk/tools/bin/sdkmanager "extras;android;m2repository" "extras;google;m2repository" >> /dev/null

export OPENSSL_VERSION=1.0.2k
export CURL_VERSION=7.53.1
export BUILD_TOOLS=`sed -n "s/.*buildToolsVersion\\s*\\"\\(.*\\)\\"/\\1/p" $BUILD_DIR/build/BOINC/boinc/android/BOINC/app/build.gradle`
export COMPILE_SDK=`sed -n "s/.*compileSdkVersion\\s*\\(\\d*\\)/\\1/p" $BUILD_DIR/build/BOINC/boinc/android/BOINC/app/build.gradle`
yes | $BUILD_DIR/Android/Sdk/tools/bin/sdkmanager "build-tools;${BUILD_TOOLS}"
yes | $BUILD_DIR/Android/Sdk/tools/bin/sdkmanager "platforms;android-${COMPILE_SDK}"

export ANDROID_TC=$BUILD_DIR/Android/Toolchains
mkdir $BUILD_DIR/3rdParty
wget -O /tmp/openssl.tgz https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz
tar xzf /tmp/openssl.tgz --directory=$BUILD_DIR/3rdParty

export OPENSSL_SRC=$BUILD_DIR/3rdParty/openssl-${OPENSSL_VERSION}
wget -O /tmp/curl.tgz https://curl.haxx.se/download/curl-${CURL_VERSION}.tar.gz
tar xzf /tmp/curl.tgz --directory=$BUILD_DIR/3rdParty

export CURL_SRC=$BUILD_DIR/3rdParty/curl-${CURL_VERSION}

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
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        --cache_dir)
        cache_dir="$2"
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
    PREFIX="$(pwd)/3rdParty/buildCache/android"
    cd android/
fi

mkdir -p "${PREFIX}"

if [ "${doclean}" = "yes" ]; then
    echo "cleaning cache"
    rm -rf "${PREFIX}"
    mkdir -p "${PREFIX}"
fi

if [[ $arch == "arm" ]]; then
    ./build_androidtc_arm.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi
    ./build_openssl_arm.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi
    ./build_curl_arm.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi
    ./build_boinc_arm.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi

    exit 0
fi

if [[ $arch == "arm64" ]]; then
    ./build_androidtc_arm64.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi
    ./build_openssl_arm64.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi
    ./build_curl_arm64.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi
    ./build_boinc_arm64.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi

    exit 0
fi

if [[ $arch == "mips" ]]; then
    ./build_androidtc_mips.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi
    ./build_openssl_mips.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi
    ./build_curl_mips.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi
    ./build_boinc_mips.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi

    exit 0
fi

if [[ $arch == "mips64" ]]; then
    ./build_androidtc_mips64.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi
    ./build_openssl_mips64.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi
    ./build_curl_mips64.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi
    ./build_boinc_mips64.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi

    exit 0
fi

if [[ $arch == "x86" ]]; then
    ./build_androidtc_x86.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi
    ./build_openssl_x86.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi
    ./build_curl_x86.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi
    ./build_boinc_x86.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi

    exit 0
fi

if [[ $arch == "x86_64" ]]; then
    ./build_androidtc_x86_64.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi
    ./build_openssl_x86_64.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi
    ./build_curl_x86_64.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi
    ./build_boinc_x86_64.sh --cache_dir "${PREFIX}"
    if [ $? -ne 0 ]; then exit 1; fi

    exit 0
fi

echo "unknown architecture: $arch"
exit 1
