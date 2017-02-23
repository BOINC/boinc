#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2015 University of California
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

## Travis-CI support script to build BOINC on Macintosh
## Downloads third party libraries needed for building BOINC on Macintosh
## if needed.
## The directory is cached by Travis-CI for all subsequent runs of the VM

# change to correct working directory because the script is called like: ./build/buildMacDependencies.sh
mkdir -p ./build/mac
cd ./build/mac

# delete old versions and trigger a new build
old_versions=(c-ares-1.10.0 openssl-1.0.1j curl-7.39.0 sqlite-autoconf-3080300 freetype-2.4.10)
for old in ${old_versions[@]}; do
    if [ -d "${old}" ]; then
        rm -rf ./${old}
        rm -f build_complete
    fi
done

rm -f build_complete

if [ -e "build_complete" ]; then
    exit 0
fi

if [ ! -d "c-ares-1.11.0" ]; then
  wget http://c-ares.haxx.se/download/c-ares-1.11.0.tar.gz
  tar -xf c-ares-1.11.0.tar.gz
  rm c-ares-1.11.0.tar.gz
fi

if [ ! -d "curl-7.50.2" ]; then
  wget http://curl.haxx.se/download/curl-7.50.2.tar.gz
  tar -xf curl-7.50.2.tar.gz
  rm curl-7.50.2.tar.gz
fi

if [ ! -d "openssl-1.1.0" ]; then
  wget https://www.openssl.org/source/openssl-1.1.0.tar.gz
  tar -xf openssl-1.1.0.tar.gz
  rm openssl-1.1.0.tar.gz
fi

if [ ! -d "wxWidgets-3.0.0" ]; then
  wget http://sourceforge.net/projects/wxwindows/files/3.0.0/wxWidgets-3.0.0.tar.bz2
  tar -xf wxWidgets-3.0.0.tar.bz2
  rm wxWidgets-3.0.0.tar.bz2
fi

if [ ! -d "sqlite-autoconf-3110000" ]; then
  wget http://www.sqlite.org/2016/sqlite-autoconf-3110000.tar.gz
  tar -xf sqlite-autoconf-3110000.tar.gz
  rm sqlite-autoconf-3110000.tar.gz
fi

if [ ! -d "freetype-2.6.2" ]; then
  wget http://sourceforge.net/projects/freetype/files/freetype2/2.6.2/freetype-2.6.2.tar.bz2
  tar -xf freetype-2.6.2.tar.bz2
  rm freetype-2.6.2.tar.bz2
fi

if [ ! -d "ftgl-2.1.3~rc5" ]; then
  wget http://sourceforge.net/projects/ftgl/files/FTGL%20Source/2.1.3%7Erc5/ftgl-2.1.3-rc5.tar.gz
  tar -xf ftgl-2.1.3-rc5.tar.gz
  rm ftgl-2.1.3-rc5.tar.gz
fi

## This script checks if a cached version is available and builds one if not.
cd ../../mac_build
source setupForBoinc.sh

if [ $? -eq 0 ]; then
    touch ../build/mac/build_complete
fi

# change back to root directory
cd ..
