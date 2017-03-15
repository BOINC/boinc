#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2017 University of California
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
## The build is done in 3rdParty/mac and usable files are installed to 3rdParty/buildCache/mac
## in order to keep the cache as small as possible
## The 3rdParty/buildCache/mac directory can be cached by CI systems for all subsequent builds

## This script should do what mac_build/setupForBOINC.sh does but install to 3rdParty/buildCache/mac

## Usage:
##     ./3rdParty/buildMacDependencies.sh [--clean] [--cache_dir PATH] [--debug]
##
## --clean will force a full rebuild of the cache.
## --cache_dir must be an absolute path where the libraries are installed to (default: 3rdParty/buildCache/mac)
## --debug will also build the debug versions of wxWidgets and BOINC Manager (Cache will be larger)
##

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

# check working directory because the script needs to be called like: ./3rdParty/buildMacDependencies.sh
if [ ! -d "3rdParty" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

ROOTDIR=$(pwd)
cache_dir=""
doclean=""
wxoption="--nodebug"
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
        --debug)
        wxoption=""
        ;;
        *)
        echo "unrecognized option $key"
        ;;
    esac
    shift # past argument or value
done

if [ "x$cache_dir" != "x" ]; then
    if isPathCanonical "$cache_dir"; then
        PREFIX="$cache_dir"
    else
        echo "cache_dir must be an absolute path without ./ or ../ in it"
        exit 1
    fi
else
    PREFIX="$(pwd)/3rdParty/buildCache/mac"
fi

download_and_build() {
    cd "${ROOTDIR}/3rdParty/mac"
    DIRNAME="${1}"
    FILENAME="${2}"
    DLURL="${3}"
    BUILDSCRIPT="${4}"
    FLAGFILE="${PREFIX}/${DIRNAME}_done"
    if [ -e "${FLAGFILE}" ]; then
        echo "${DIRNAME} seems already to be present in ${PREFIX}"
        return 0
    fi
    if [ ! -d ${DIRNAME} ]; then
        if [ ! -e ${FILENAME} ]; then
            wget ${DLURL}
        fi
        tar -xf ${FILENAME}
    fi
    cd ${DIRNAME}
    source ${BUILDSCRIPT} --prefix ${PREFIX}
    if [ $? -ne 0 ]; then exit 1; fi
    cd ../..
    touch ${FLAGFILE}
}

mkdir -p 3rdParty/mac
mkdir -p ${PREFIX}
cd "${ROOTDIR}/3rdParty/mac"

if [ "${doclean}" = "yes" ]; then
    echo "cleaning cache"
    rm -rf ${PREFIX}/* ${PREFIX}/*.*
fi

#download_and_build $DIRNAME $FILENAME $DOWNLOADURL $BUILDSCRIPT
download_and_build "openssl-1.1.0" "openssl-1.1.0.tar.gz" "https://www.openssl.org/source/openssl-1.1.0.tar.gz" "${ROOTDIR}/mac_build/buildopenssl.sh"
download_and_build "c-ares-1.11.0" "c-ares-1.11.0.tar.gz" "http://c-ares.haxx.se/download/c-ares-1.11.0.tar.gz" "${ROOTDIR}/mac_build/buildc-ares.sh"
download_and_build "curl-7.50.2" "curl-7.50.2.tar.gz" "http://curl.haxx.se/download/curl-7.50.2.tar.gz" "${ROOTDIR}/mac_build/buildcurl.sh"
download_and_build "wxWidgets-3.0.0" "wxWidgets-3.0.0.tar.bz2" "http://sourceforge.net/projects/wxwindows/files/3.0.0/wxWidgets-3.0.0.tar.bz2" "${ROOTDIR}/mac_build/buildWxMac.sh ${wxoption}"
download_and_build "sqlite-autoconf-3110000" "sqlite-autoconf-3110000.tar.gz" "http://www.sqlite.org/2016/sqlite-autoconf-3110000.tar.gz" "${ROOTDIR}/mac_build/buildsqlite3.sh"
download_and_build "freetype-2.6.2" "freetype-2.6.2.tar.bz2" "http://sourceforge.net/projects/freetype/files/freetype2/2.6.2/freetype-2.6.2.tar.bz2" "${ROOTDIR}/mac_build/buildfreetype.sh"
download_and_build "ftgl-2.1.3~rc5" "ftgl-2.1.3-rc5.tar.gz" "http://sourceforge.net/projects/ftgl/files/FTGL%20Source/2.1.3%7Erc5/ftgl-2.1.3-rc5.tar.gz" "${ROOTDIR}/mac_build/buildFTGL.sh"

# change back to root directory
cd ${ROOTDIR}
