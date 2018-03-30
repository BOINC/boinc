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

if [ "x$TRAVIS_BUILD_DIR" != "x" ]; then
    export BUILD_DIR="$TRAVIS_BUILD_DIR"
else
    export BUILD_DIR=".."
fi

umake android android-studio --accept-license $HOME/Android/Android-Studio

export ANDROID_HOME=$HOME/Android/Sdk
umake android android-sdk --accept-license $HOME/Android/Sdk

export NDK_ROOT=$HOME/Android/Ndk
umake android android-ndk --accept-license $HOME/Android/Ndk

yes | $HOME/Android/Sdk/tools/bin/sdkmanager --update >> /dev/null
yes | $HOME/Android/Sdk/tools/bin/sdkmanager "extras;android;m2repository" "extras;google;m2repository" >> /dev/null

export BUILD_TOOLS=`sed -n "s/.*buildToolsVersion\\s*\\"\\(.*\\)\\"/\\1/p" $BUILD_DIR/android/BOINC/app/build.gradle`
export COMPILE_SDK=`sed -n "s/.*compileSdkVersion\\s*\\(\\d*\\)/\\1/p" $BUILD_DIR/android/BOINC/app/build.gradle`
yes | $HOME/Android/Sdk/tools/bin/sdkmanager "build-tools;${BUILD_TOOLS}"
yes | $HOME/Android/Sdk/tools/bin/sdkmanager "platforms;android-${COMPILE_SDK}"

export ANDROID_TC=$HOME/Android/Toolchains

mkdir -p $HOME/3rdParty

export OPENSSL_VERSION=1.0.2k
wget -O /tmp/openssl.tgz https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz
tar xzf /tmp/openssl.tgz --directory=$HOME/3rdParty
export OPENSSL_SRC=$HOME/3rdParty/openssl-${OPENSSL_VERSION}

export CURL_VERSION=7.53.1
wget -O /tmp/curl.tgz https://curl.haxx.se/download/curl-${CURL_VERSION}.tar.gz
tar xzf /tmp/curl.tgz --directory=$HOME/3rdParty
export CURL_SRC=$HOME/3rdParty/curl-${CURL_VERSION}
