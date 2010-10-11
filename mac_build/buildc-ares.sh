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
# Script to build Macintosh Universal Binary library of c-ares-1.6.0 for
# use in building BOINC.
#
# Note: reverted to c-ares 1.6.0 from 1.7.0 because the newer c-ares has 
# problems resolving host names on OS 10.6 with default settings when used 
# with AT&T U-Verse 2Wire gateway routers and Airport.
#
# by Charlie Fenton 7/21/06
# Updated 12/3/09 for OS 10.6 Snow Leopard and XCode 3.2.1
# Updated 10/11/10
#
## In Terminal, CD to the c-ares-1.6.0 directory.
##     cd [path]/c-ares-1.6.0/
## then run this script:
##     source [path]/buildc-ares.sh [ -clean ]
##
## the -clean argument will force a full rebuild.
##

if [ "$1" != "-clean" ]; then
    if [ -f .libs/libcares_ppc.a ] && [ -f .libs/libcares_i386.a ] && [ -f .libs/libcares.a ]; then
        echo "c-ares-1.6.0 already built"
        return 0
    fi
fi

if [ ! -d /Developer/SDKs/MacOSX10.4u.sdk/ ]; then
    echo "ERROR: System 10.4u SDK is missing.  For details, see build instructions at"
    echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or http://boinc.berkeley.edu/trac/wiki/MacBuild"
    return 1
fi

export PATH=/usr/local/bin:$PATH

rm -f .libs/libcares.a
rm -f .libs/libcares_ppc.a
rm -f .libs/libcares_i386.a

export CC=/usr/bin/gcc-4.0;export CXX=/usr/bin/g++-4.0
export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -Wl,-syslibroot,/Developer/SDKs/MacOSX10.4u.sdk -arch ppc -DMAC_OS_X_VERSION_MAX_ALLOWED=1030 -DMAC_OS_X_VERSION_MIN_REQUIRED=1030"
export CPPFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch ppc"
export CFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch ppc"
export SDKROOT="/Developer/SDKs/MacOSX10.4u.sdk"
export MACOSX_DEPLOYMENT_TARGET=10.3

./configure --enable-shared=NO prefix=/tmp/installed-c-ares --host=ppc
if [  $? -ne 0 ]; then return 1; fi


make clean

make
if [  $? -ne 0 ]; then return 1; fi
# c-ares configure creates a different ares_build.h file for each architecture
cp -f ares_build.h ares_build_ppc.h
mv -f .libs/libcares.a libcares_ppc.a

make clean
if [  $? -ne 0 ]; then return 1; fi

export PATH=/usr/local/bin:$PATH
export CC=/usr/bin/gcc-4.0;export CXX=/usr/bin/g++-4.0
export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -Wl,-syslibroot,/Developer/SDKs/MacOSX10.4u.sdk -arch i386 -DMAC_OS_X_VERSION_MAX_ALLOWED=1030 -DMAC_OS_X_VERSION_MIN_REQUIRED=1030"
export CPPFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386"
export CFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386"
export SDKROOT="/Developer/SDKs/MacOSX10.4u.sdk"
export MACOSX_DEPLOYMENT_TARGET=10.4

./configure --enable-shared=NO prefix=/tmp/installed-c-ares --host=i386
if [  $? -ne 0 ]; then return 1; fi

make
if [  $? -ne 0 ]; then return 1; fi
# c-ares configure creates a different ares_build.h file for each architecture
cp -f ares_build.h ares_build_i386.h

export CC="";export CXX=""
export LDFLAGS=""
export CPPFLAGS=""
export CFLAGS=""
export SDKROOT=""

mv -f .libs/libcares.a libcares_i386.a

mv -f libcares_ppc.a .libs/
mv -f libcares_i386.a .libs/
lipo -create .libs/libcares_i386.a .libs/libcares_ppc.a -output .libs/libcares.a
if [  $? -ne 0 ]; then return 1; fi

return 0
