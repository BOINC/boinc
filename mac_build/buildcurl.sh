#!/bin/sh

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2008 University of California
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
# Script to build Macintosh 32-bit Intel library of curl-7.26.0 for
# use in building BOINC.
#
# by Charlie Fenton 7/21/06
# Updated 12/3/09 for OS 10.7 Lion and XCode 4.2
# Updated 6/25/12 for curl 7.26.0
#
## In Terminal, CD to the curl-7.26.0 directory.
##     cd [path]/curl-7.26.0/
## then run this script:
##     source [path]/buildcurl.sh [ -clean ]
##
## the -clean argument will force a full rebuild.
##

if [ "$1" != "-clean" ]; then
    if [ -f lib/.libs/libcurl.a ]; then
        echo "curl-7.26.0 already built"
        return 0
    fi
fi

if [ ! -d /Developer/SDKs/MacOSX10.6.sdk/ ]; then
    echo "ERROR: System 10.6 SDK is missing.  For details, see build instructions at"
    echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or http://boinc.berkeley.edu/trac/wiki/MacBuild"
    return 1
fi

export PATH=/usr/local/bin:$PATH

CURL_DIR=`pwd`
# curl configure and make expect a path to _installed_ c-ares-1.9.1
# so temporarily install c-ares at a path that does not contain spaces.
cd ../c-ares-1.9.1
make install 
cd "${CURL_DIR}"


rm -f lib/.libs/libcurl.a

if [  $? -ne 0 ]; then return 1; fi

export PATH=/usr/local/bin:$PATH
export CC=/usr/bin/llvm-gcc-4.2;export CXX=/usr/bin/llvm-g++-4.2
export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.6.sdk -Wl,-syslibroot,/Developer/SDKs/MacOSX10.t.sdk -arch i386"
export CPPFLAGS="-isysroot /Developer/SDKs/MacOSX10.6.sdk -arch i386 -DMAC_OS_X_VERSION_MAX_ALLOWED=1030 -DMAC_OS_X_VERSION_MIN_REQUIRED=1030"
export CFLAGS="-isysroot /Developer/SDKs/MacOSX10.6.sdk -arch i386 -DMAC_OS_X_VERSION_MAX_ALLOWED=1030 -DMAC_OS_X_VERSION_MIN_REQUIRED=1030"
export SDKROOT="/Developer/SDKs/MacOSX10.6.sdk"
export MACOSX_DEPLOYMENT_TARGET=10.4

./configure --enable-shared=NO --enable-ares=/tmp/installed-c-ares --host=i386
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
