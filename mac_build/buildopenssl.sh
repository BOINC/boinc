#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2019 University of California
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
#
# Script to build Macintosh 64-bit Intel openssl libraries
# libcrypto.a and libssl.a for use in building BOINC.
#
# by Charlie Fenton 6/25/12
# Updated 7/10/12 for Xcode 4.3 and later which are not at a fixed address
# Updated 7/30/13 for openssl-1.0.1e
# Updated 2/12/14 for openssl-1.0.1f
# Updated 4/14/14 for openssl-1.0.1g
# Updated 6/6/14 for openssl-1.0.1h
# Updated 9/2/14 for bulding openssl as 64-bit binary
# Updated 6/6/14 for openssl-1.0.1j
# Updated 12/11/15 for openssl-1.0.2e
# Updated 3/2/16 for openssl-1.0.2g
# Updated 9/10/16 for openssl-1.1.0
# Updated 1/25/18 for bulding openssl 1.1.0g (updated comemnts only)
# Updated 1/23/19 use libc++ instead of libstdc++ for Xcode 10 compatibility
#
## This script requires OS 10.8 or later
#
## After first installing Xcode, you must have opened Xcode and
## clicked the Install button on the dialog which appears to
## complete the Xcode installation before running this script.
#
## Where x.xx.xy is the openssl version number:
## In Terminal, CD to the openssl-x.xx.xy directory.
##     cd [path]/openssl-x.xx.xy/
## then run this script:
##     source [path]/buildopenssl.sh [ -clean ] [--prefix PATH]
##
## the -clean argument will force a full rebuild.
## if --prefix is given as absolute path the library is installed into there
## use -q or --quiet to redirect build output to /dev/null instead of /dev/stdout
##

doclean=""
stdout_target="/dev/stdout"
lprefix=""
libPath="."
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -clean|--clean)
        doclean="yes"
        ;;
        -prefix|--prefix)
        lprefix="$2"
        libPath="${lprefix}/lib"
        shift
        ;;
        -q|--quiet)
        stdout_target="/dev/null"
        ;;
    esac
    shift # past argument or value
done

if [ "${doclean}" != "yes" ]; then
    if [ -f ${libPath}/libssl.a ] && [ -f ${libPath}/libcrypto.a ]; then
        cwd=$(pwd)
        dirname=${cwd##*/}
        echo "${dirname} already built"
        return 0
    fi
fi

export PATH=/usr/local/bin:$PATH

GCCPATH=`xcrun -find gcc`
if [ $? -ne 0 ]; then
    echo "ERROR: can't find gcc compiler"
    return 1
fi

GPPPATH=`xcrun -find g++`
if [ $? -ne 0 ]; then
    echo "ERROR: can't find g++ compiler"
    return 1
fi

MAKEPATH=`xcrun -find make`
if [ $? -ne 0 ]; then
    echo "ERROR: can't find make tool"
    return 1
fi

TOOLSPATH1=${MAKEPATH%/make}

ARPATH=`xcrun -find ar`
if [ $? -ne 0 ]; then
    echo "ERROR: can't find ar tool"
    return 1
fi

TOOLSPATH2=${ARPATH%/ar}

SDKPATH=`xcodebuild -version -sdk macosx Path`

export PATH="${TOOLSPATH1}":"${TOOLSPATH2}":/usr/local/bin:$PATH

if [ -d "${libPath}" ]; then
    rm -f ${libPath}/libssl.a
    rm -f ${libPath}/libcrypto.a
fi

export CC="${GCCPATH}";export CXX="${GPPPATH}"
export CPPFLAGS=""
export LDFLAGS="-Wl,-sysroot,${SDKPATH},-syslibroot,${SDKPATH},-arch,x86_64"
export CXXFLAGS="-isysroot ${SDKPATH} -arch x86_64 -stdlib=libc++ -DMAC_OS_X_VERSION_MAX_ALLOWED=1070 -DMAC_OS_X_VERSION_MIN_REQUIRED=1070"
export CFLAGS="-isysroot ${SDKPATH} -arch x86_64 -DMAC_OS_X_VERSION_MAX_ALLOWED=1070 -DMAC_OS_X_VERSION_MIN_REQUIRED=1070"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.7
export LIBRARY_PATH="${SDKPATH}/usr/lib"

if [ "x${lprefix}" != "x" ]; then
    ./configure --prefix=${lprefix} no-shared darwin64-x86_64-cc
    if [ $? -ne 0 ]; then return 1; fi
else
    ./configure no-shared darwin64-x86_64-cc
    if [ $? -ne 0 ]; then return 1; fi
fi

if [ "${doclean}" = "yes" ]; then
    make clean
fi

make 1>$stdout_target
if [ $? -ne 0 ]; then return 1; fi

if [ "x${lprefix}" != "x" ]; then
    make install 1>$stdout_target
    if [ $? -ne 0 ]; then return 1; fi
fi

lprefix=""
export CC="";export CXX=""
export LDFLAGS=""
export CXXFLAGS=""
export CFLAGS=""
export SDKROOT=""

return 0
