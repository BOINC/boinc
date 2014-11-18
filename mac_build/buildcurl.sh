#!/bin/sh

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2014 University of California
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
# Script to build Macintosh 32-bit Intel library of curl-7.35.0 for
# use in building BOINC.
#
# by Charlie Fenton 7/21/06
# Updated 12/3/09 for OS 10.7 Lion and XCode 4.2
# Updated 6/25/12 for curl 7.26.0
# Updated 7/10/12 for Xcode 4.3 and later which are not at a fixed address
# Updated 2/11/14 for curl 7.35.0 with c-ares 1.10.0
# Updated 9/2/14 for bulding curl as 64-bit binary
# Updated 11/17/14 for curl 7.39.0 with c-ares 1.10.0
#
## This script requires OS 10.6 or later
## This script requires OS 10.6 or later
#
## If you drag-install Xcode 4.3 or later, you must have opened Xcode 
## and clicked the Install button on the dialog which appears to 
## complete the Xcode installation before running this script.
#
## In Terminal, CD to the curl-7.39.0 directory.
##     cd [path]/curl-7.39.0/
## then run this script:
##     source [path]/buildcurl.sh [ -clean ]
##
## the -clean argument will force a full rebuild.
##

if [ "$1" != "-clean" ]; then
    if [ -f lib/.libs/libcurl.a ]; then
        echo "curl-7.39.0 already built"
        return 0
    fi
fi

export PATH=/usr/local/bin:$PATH

GCCPATH=`xcrun -find gcc`
if [  $? -ne 0 ]; then
    echo "ERROR: can't find gcc compiler"
    return 1
fi

GPPPATH=`xcrun -find g++`
if [  $? -ne 0 ]; then
    echo "ERROR: can't find g++ compiler"
    return 1
fi

MAKEPATH=`xcrun -find make`
if [  $? -ne 0 ]; then
    echo "ERROR: can't find make tool"
    return 1
fi

TOOLSPATH1=${MAKEPATH%/make}

ARPATH=`xcrun -find ar`
if [  $? -ne 0 ]; then
    echo "ERROR: can't find ar tool"
    return 1
fi

TOOLSPATH2=${ARPATH%/ar}

export PATH="${TOOLSPATH1}":"${TOOLSPATH2}":/usr/local/bin:$PATH

SDKPATH=`xcodebuild -version -sdk macosx Path`

CURL_DIR=`pwd`
# curl configure and make expect a path to _installed_ c-ares-1.10.0
# so temporarily install c-ares at a path that does not contain spaces.
cd ../c-ares-1.10.0
make install 
cd "${CURL_DIR}"


rm -f lib/.libs/libcurl.a

if [  $? -ne 0 ]; then return 1; fi

export PATH=/usr/local/bin:$PATH
export CC="${GCCPATH}";export CXX="${GPPPATH}"
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64"
export CPPFLAGS="-isysroot ${SDKPATH} -arch x86_64"
export CFLAGS="-isysroot ${SDKPATH} -arch x86_64"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.5
export MAC_OS_X_VERSION_MAX_ALLOWED=1050
export MAC_OS_X_VERSION_MIN_REQUIRED=1050

./configure --enable-shared=NO --enable-ares=/tmp/installed-c-ares --host=x86_64
if [  $? -ne 0 ]; then return 1; fi

if [ "$1" = "-clean" ]; then
    make clean
fi

make
if [  $? -ne 0 ]; then return 1; fi

export CC="";export CXX=""
export LDFLAGS=""
export CPPFLAGS=""
export CFLAGS=""
export SDKROOT=""

# Delete temporarily installed c-ares.
rm -Rf /tmp/installed-c-ares/

return 0
