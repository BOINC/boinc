#!/bin/bash
set -x

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

# change to correct working directory because the script is called like: ./mac3rdParty/buildMacTravis.sh
cd ./mac3rdParty

# if the versioning changes try to delete the old directory and change .travis.yml to cache the new directory


if [ ! -d "c-ares-1.10.0" ]; then
  wget http://c-ares.haxx.se/download/c-ares-1.10.0.tar.gz
  tar -xf c-ares-1.10.0.tar.gz
fi

if [ ! -d "curl-7.39.0" ]; then
  wget http://curl.haxx.se/download/curl-7.39.0.tar.gz
  tar -xf curl-7.39.0.tar.gz
fi

if [ ! -d "openssl-1.0.1j" ]; then
  wget http://www.openssl.org/source/openssl-1.0.1j.tar.gz
  tar -xf openssl-1.0.1j.tar.gz
fi

if [ ! -d "wxWidgets-3.0.0" ]; then
  wget http://sourceforge.net/projects/wxwindows/files/3.0.0/wxWidgets-3.0.0.tar.bz2/
  tar -xf wxWidgets-3.0.0.tar.bz2
fi

if [ ! -d "sqlite-autoconf-3080300" ]; then
  wget http://www.sqlite.org/2014/sqlite-autoconf-3080300.tar.gz
  tar -xf sqlite-autoconf-3080300.tar.gz
fi

if [ ! -d "freetype-2.4.10" ]; then
  wget http://sourceforge.net/projects/freetype/files/freetype2/2.4.10/freetype-2.4.10.tar.bz2
  tar -xf freetype-2.4.10.tar.bz2
fi

if [ ! -d "ftgl-2.1.3~rc5" ]; then
  wget http://sourceforge.net/projects/ftgl/files/FTGL%20Source/2.1.3%7Erc5/ftgl-2.1.3-rc5.tar.gz
  tar -xf ftgl-2.1.3-rc5.tar.gz
fi

## This script checks if a cached version is available and builds one if not.
cd ../mac_build
source setupForBoinc.sh

if [  $? -eq 0 ]; then
    source BuildMacBOINC.sh
fi
