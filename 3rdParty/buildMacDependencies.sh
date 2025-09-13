#!/bin/bash

# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2025 University of California
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

## IMPORTANT: This script is used only by GitHub Continuous Integration. Do not use it for
## building BOINC on a Macintosh computer! Use mac_build/setupForBOINC.sh instead.

## support script to build BOINC dependencies on Macintosh
## This script checks if cached versions are available and builts them if not.
## The build is done in 3rdParty/mac and usable files are installed to 3rdParty/buildCache/mac
## in order to keep the cache as small as possible
## The 3rdParty/buildCache directory can be cached by CI systems for all subsequent builds

## This script should do what mac_build/setupForBOINC.sh does but install to 3rdParty/buildCache/mac

## Usage:
##     ./3rdParty/buildMacDependencies.sh [--clean] [--cache_dir PATH] [--debug]
##
## --clean will force a full rebuild of the cache.
## --cache_dir must be an absolute path where the libraries are installed to (default: 3rdParty/buildCache/mac)
##             ATTENTION: don't set this to /usr or /var this is an internal cache and it should stay in the source path
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
extra_options=""
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
        -q|--quiet)
        extra_options="${extra_options} --quiet"
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
    PREFIX="$(pwd)/3rdParty/buildCache/mac"
fi

download_and_build() {
    cd "${ROOTDIR}/3rdParty/mac" || exit 1
    DIRNAME="${1}"
    FILENAME="${2}"
    DLURL="${3}"
    BUILDSCRIPT="${4}"
    PRODUCTNAME="${5}"
    ARCHS="${6}"
    FLAGFILE="${PREFIX}/${DIRNAME}_done"
    doClean=""
    if [ -e "${FLAGFILE}" ]; then
        lipo "${PREFIX}/lib/${PRODUCTNAME}" -verify_arch ${ARCHS}
        if [ $? -eq 0 ]; then
            echo "${DIRNAME} seems already to be present in ${PREFIX}"
            return 0
        else
            # already built but not for correct architectures so force rebuild
            doClean="-clean"
        fi
    else
        # tell subsequent scripts to build everything from scratch
        doClean="-clean"
        # delete any FILEFLAGS for other versions of this library built previously
        BASENAME="${DIRNAME%-*}"
        rm -f "${PREFIX}/${BASENAME}"*
        # delete any previous build of this library (may be redundant with -clean)
        rm -f "${PREFIX}/lib/${PRODUCTNAME}"
    fi
    if [ ! -d ${DIRNAME} ]; then
        if [ ! -e ${FILENAME} ]; then
            wget ${DLURL}
        fi
        tar -xf ${FILENAME}
    fi
    cd ${DIRNAME} || exit 1
    source ${BUILDSCRIPT} --prefix "${PREFIX}" ${extra_options} ${doClean}
    if [ $? -ne 0 ]; then exit 1; fi
    cd ../.. || exit 1
    touch ${FLAGFILE}
}

mkdir -p 3rdParty/mac
mkdir -p "${PREFIX}"
cd "${ROOTDIR}/3rdParty/mac" || exit 1

if [ "${doclean}" = "yes" ]; then
    echo "cleaning cache"
    rm -rf "${PREFIX}"
    mkdir -p "${PREFIX}"
fi

# this will pull in the variables used below
source "${ROOTDIR}/mac_build/dependencyNames.sh"

#download_and_build $DIRNAME $FILENAME $DOWNLOADURL $BUILDSCRIPT $PRODUCTNAME $ARCHS
download_and_build "${opensslDirName}" "${opensslFileName}" "${opensslURL}" "${ROOTDIR}/mac_build/buildopenssl.sh" "libssl.a" "x86_64 arm64"
download_and_build "${caresDirName}" "${caresFileName}" "${caresURL}" "${ROOTDIR}/mac_build/buildc-ares.sh" "libcares.a" "x86_64 arm64"
download_and_build "${curlDirName}" "${curlFileName}" "${curlURL}" "${ROOTDIR}/mac_build/buildcurl.sh" "libcurl.a" "x86_64 arm64"
download_and_build "${wxWidgetsDirName}" "${wxWidgetsFileName}" "${wxWidgetsURL}" "${ROOTDIR}/mac_build/buildWxMac.sh ${wxoption}" "libwx_osx_cocoa_static.a" "x86_64 arm64"
download_and_build "${freetypeDirName}" "${freetypeFileName}" "${freetypeURL}" "${ROOTDIR}/mac_build/buildfreetype.sh" "libfreetype.a" "x86_64 arm64"
download_and_build "${ftglDirName}" "${ftglFileName}" "${ftglURL}" "${ROOTDIR}/mac_build/buildFTGL.sh" "libftgl.a" "x86_64 arm64"
download_and_build "${zipDirName}" "${zipFileName}" "${zipURL}" "${ROOTDIR}/mac_build/buildlibzip.sh" "libzip.a" "x86_64 arm64"

# change back to root directory
cd ${ROOTDIR} || exit 1
