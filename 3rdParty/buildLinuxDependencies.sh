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

## support script to build a specific wxWidgets version for linux
## This script checks if a cached version is available and builts one if not.
## The build is done in 3rdParty/linux and usable files are installed to 3rdParty/buildCache/linux
## in order to keep the cache as small as possible
## The 3rdParty/buildCache directory can be cached by CI systems for all subsequent builds

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

# check working directory because the script needs to be called like: ./3rdParty/buildLinuxDependencies.sh
if [ ! -d "3rdParty" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

ROOTDIR=$(pwd)
cache_dir=""
doclean=""
wxoption=""
build_config="Release"
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
        wxoption="--debug ${wxoption}"
        build_config="Debug"
        ;;
        --disable-webview)
        wxoption="--disable-webview ${wxoption} "
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
    PREFIX="$(pwd)/3rdParty/buildCache/linux"
fi

if [ -f "${PREFIX}/build-config" ]; then
    cur_config=$(<${PREFIX}/build-config)
    if [ "${cur_config}" != "${build_config}" ]; then
        doclean="yes"
        wxoption="${wxoption} --clean"
    fi
else
    doclean="yes"
    wxoption="${wxoption} --clean"
fi

download_and_build() {
    cd "${ROOTDIR}/3rdParty/linux" || exit 1
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
    cd ${DIRNAME} || exit 1
    source ${BUILDSCRIPT} --prefix ${PREFIX}
    if [ $? -ne 0 ]; then exit 1; fi
    cd ../.. || exit 1
    touch ${FLAGFILE}
}

mkdir -p 3rdParty/linux
mkdir -p ${PREFIX}
cd "${ROOTDIR}/3rdParty/linux" || exit 1

if [ "${doclean}" = "yes" ]; then
    echo "cleaning cache"
    rm -rf ${PREFIX}
    mkdir -p ${PREFIX}
    echo ${build_config} >${PREFIX}/build-config
fi

wx_ver="3.2.6"
#download_and_build $DIRNAME $FILENAME $DOWNLOADURL $BUILDSCRIPT
download_and_build "wxWidgets-$wx_ver" "wxWidgets-$wx_ver.tar.bz2" "https://github.com/wxWidgets/wxWidgets/releases/download/v$wx_ver/wxWidgets-$wx_ver.tar.bz2" "${ROOTDIR}/3rdParty/buildWxLinux.sh ${wxoption}"

# change back to root directory
cd ${ROOTDIR} || exit 1
