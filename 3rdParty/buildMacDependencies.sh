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

## support script to build BOINC dependencies on Macintosh
## This script checks if cached versions are available and builts them if not.
## The build is done in 3rdParty/mac and usable files are installed to install/mac
## in order to keep the cache as small as possible
## The install directory is cached by Travis-CI for all subsequent runs of the VM

## This script should do what mac_build/setupForBOINC.sh does but install to install/mac

# check working directory because the script needs to be called like: ./3rdParty/buildMacDependencies.sh
if [ ! -d "3rdParty" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

SRCDIR=$(pwd)
PREFIX="$(pwd)/install/mac"

download_and_build() {
    DIRNAME="${1}"
    FILENAME="${2}"
    DLURL="${3}"
    BUILDSCRIPT="${4}"
    FLAGFILE="${PREFIX}/${DIRNAME}_done"
    if [ -e "${FLAGFILE}" ]; then
        echo "${DIRNAME} seems already to be present in install/mac"
        exit 0
    fi
    if [ ! -d ${DIRNAME} ]; then
        if [ ! -e ${FILENAME} ]; then
            wget ${DLURL}
        fi
        tar -xf ${FILENAME}
    fi
    cd ${DIRNAME}
    source ${BUILDSCRIPT}
    if [ $? -ne 0 ]; then exit 1; fi
    cd ../..
    touch ${FLAGFILE}
}

if [ ! -d "install" ]; then
    echo "install is not a directory, deleting and recreating"
    rm -f install
fi
mkdir -p 3rdParty/mac
mkdir -p install/mac
cd ./3rdParty/mac

#download_and_build $DIRNAME $FILENAME $DOWNLOADURL $BUILDSCRIPT
download_and_build "openssl-1.1.0" "openssl-1.1.0.tar.gz" "https://www.openssl.org/source/openssl-1.1.0.tar.gz" "${SRCDIR}/mac_build/buildopenssl.sh"
download_and_build "c-ares-1.11.0" "c-ares-1.11.0.tar.gz" "http://c-ares.haxx.se/download/c-ares-1.11.0.tar.gz" "${SRCDIR}/mac_build/buildc-ares.sh"
download_and_build "curl-7.50.2" "curl-7.50.2.tar.gz" "http://curl.haxx.se/download/curl-7.50.2.tar.gz" "${SRCDIR}/mac_build/buildcurl.sh"
#download_and_build "wxWidgets-3.0.0" "wxWidgets-3.0.0.tar.bz2" "http://sourceforge.net/projects/wxwindows/files/3.0.0/wxWidgets-3.0.0.tar.bz2" "${SRCDIR}/mac_build/buildWxMac.sh"
download_and_build "sqlite-autoconf-3110000" "sqlite-autoconf-3110000.tar.gz" "http://www.sqlite.org/2016/sqlite-autoconf-3110000.tar.gz" "${SRCDIR}/mac_build/buildsqlite3.sh"
download_and_build "freetype-2.6.2" "freetype-2.6.2.tar.bz2" "http://sourceforge.net/projects/freetype/files/freetype2/2.6.2/freetype-2.6.2.tar.bz2" "${SRCDIR}/mac_build/buildfreetype.sh"
download_and_build "ftgl-2.1.3~rc5" "ftgl-2.1.3-rc5.tar.gz" "http://sourceforge.net/projects/ftgl/files/FTGL%20Source/2.1.3%7Erc5/ftgl-2.1.3-rc5.tar.gz" "${SRCDIR}/mac_build/buildFTGL.sh"

# change back to root directory
cd ..
