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

# check working directory because the script needs to be called like: ./3rdParty/getAndroidDependencies.sh
if [ ! -d "3rdParty" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

wget -O /tmp/python3-argcomplete_0.8.1-1ubuntu2_all.deb http://ftp.ubuntu.com/ubuntu/ubuntu/pool/universe/p/python-argcomplete/python3-argcomplete_0.8.1-1ubuntu2_all.deb
sudo dpkg -i /tmp/python3-argcomplete_0.8.1-1ubuntu2_all.deb

sudo add-apt-repository ppa:ubuntu-desktop/ubuntu-make -y
sudo apt-get update
sudo apt-get --assume-yes install ubuntu-make # git automake libtool
# sudo update-locale LC_ALL=en_US.UTF-8

umake android android-studio --accept-license $HOME/Android/Android-Studio
printf "\n# umake fix-up\nexport ANDROID_HOME=\$HOME/Android/Sdk\n" >> $HOME/.profile
umake android android-sdk --accept-license $HOME/Android/Sdk
printf "\n# umake fix-up\nexport NDK_ROOT=\$HOME/Android/Ndk\n" >> $HOME/.profile
umake android android-ndk --accept-license $HOME/Android/Ndk
yes | $HOME/Android/Sdk/tools/bin/sdkmanager --update >> /tmp/umake.log
yes | $HOME/Android/Sdk/tools/bin/sdkmanager "extras;android;m2repository" "extras;google;m2repository" >> /tmp/umake.log
# mkdir $HOME/Desktop
# cp $HOME/.local/share/applications/android-studio.desktop $HOME/Desktop/
# chmod +x $HOME/Desktop/android-studio.desktop
echo "start export"
export OPENSSL_VERSION=1.0.2k
export CURL_VERSION=7.53.1
export BUILD_TOOLS=`sed -n "s/.*buildToolsVersion\\s*\\"\\(.*\\)\\"/\\1/p" $HOME/build/BOINC/boinc/android/BOINC/app/build.gradle`
export COMPILE_SDK=`sed -n "s/.*compileSdkVersion\\s*\\(\\d*\\)/\\1/p" $HOME/build/BOINC/boinc/android/BOINC/app/build.gradle`
echo "sdkmanager build-tools"
yes | $HOME/Android/Sdk/tools/bin/sdkmanager "build-tools;${BUILD_TOOLS}"
echo "sdkmanager platforms"
yes | $HOME/Android/Sdk/tools/bin/sdkmanager "platforms;android-${COMPILE_SDK}"
printf "\n# Build toolchains\nexport ANDROID_TC=\$HOME/Android/Toolchains\n" >> $HOME/.profile
echo "3rdparty"
mkdir $HOME/3rdParty
wget -O /tmp/openssl.tgz https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz
tar xzf /tmp/openssl.tgz --directory=$HOME/3rdParty
printf "\n# OpenSSL sources\nexport OPENSSL_SRC=\$HOME/3rdParty/openssl-${OPENSSL_VERSION}\n" >> $HOME/.profile
wget -O /tmp/curl.tgz https://curl.haxx.se/download/curl-${CURL_VERSION}.tar.gz
tar xzf /tmp/curl.tgz --directory=$HOME/3rdParty
printf "\n# cURL sources\nexport CURL_SRC=\$HOME/3rdParty/curl-${CURL_VERSION}\n" >> $HOME/.profile
