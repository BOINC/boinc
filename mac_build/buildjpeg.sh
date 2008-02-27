#!/bin/sh

# Berkeley Open Infrastructure for Network Computing
# http://boinc.berkeley.edu
# Copyright (C) 2005 University of California
#
# This is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation;
# either version 2.1 of the License, or (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# To view the GNU Lesser General Public License visit
# http://www.gnu.org/copyleft/lesser.html
# or write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#
#
# Script to build Macintosh Universal Binary library of jpeg-6b for
# use in building BOINC.
#
# by Charlie Fenton 12/19/07
# Updated 2/27/08
#
## In Terminal, CD to the jpeg-6b directory.
##     cd [path]/jpeg-6b/
## then run this script:
##    source [ path_to_this_script ] [ -clean ]
#
# the -clean argument will force a full rebuild.
#

AlreadyBuilt=0

if [ "$1" != "-clean" ]; then
    if [ -f libjpeg_ppc.a ] && [ -f libjpeg_i386.a ] && [ -f libjpeg.a ]; then
        AlreadyBuilt=1
    fi
fi
    
if [ -d /Developer/SDKs/MacOSX10.5.sdk/ ]; then
    # Build for x86_64 architecture if OS 10.5 SDK is present
    if [ ! -f libjpeg_x86_64.a ]; then
        AlreadyBuilt=0
    fi
fi

if [ $AlreadyBuilt -ne 0 ]; then
    echo "jpeg-6b already built"
    return 0
fi

if [ ! -d /Developer/SDKs/MacOSX10.3.9.sdk/ ]; then
    echo "ERROR: System 10.3.9 SDK is missing.  For details, see build instructions at"
    echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or http://boinc.berkeley.edu/trac/wiki/MacBuild"
    return 1
fi

if [ ! -d /Developer/SDKs/MacOSX10.4u.sdk/ ]; then
    echo "ERROR: System 10.4u SDK is missing.  For details, see build instructions at"
    echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or http://boinc.berkeley.edu/trac/wiki/MacBuild"
    return 1
fi

export PATH=/usr/local/bin:$PATH
export CC=/usr/bin/gcc-3.3;export CXX=/usr/bin/g++-3.3
export LDFLAGS="-arch ppc -D_NONSTD_SOURCE -isystem /Developer/SDKs/MacOSX10.3.9.sdk -Wl,-syslibroot,/Developer/SDKs/MacOSX10.3.9.sdk"
export CPPFLAGS="-arch ppc -D_NONSTD_SOURCE -isystem /Developer/SDKs/MacOSX10.3.9.sdk"
export CFLAGS="-arch ppc -D_NONSTD_SOURCE -isystem /Developer/SDKs/MacOSX10.3.9.sdk"
export SDKROOT="/Developer/SDKs/MacOSX10.3.9.sdk"
export MACOSX_DEPLOYMENT_TARGET=10.3

./configure --disable-shared --host=ppc
if [  $? -ne 0 ]; then return 1; fi

rm -f libjpeg_ppc.a
rm -f libjpeg_i386.a
rm -f libjpeg_x86_64.a
rm -f libjpeg.a
make clean

make -e
if [  $? -ne 0 ]; then return 1; fi
mv -f libjpeg.a libjpeg_ppc.a

make clean
if [  $? -ne 0 ]; then return 1; fi

##export PATH=/usr/local/bin:$PATH
export CC=/usr/bin/gcc-4.0;export CXX=/usr/bin/g++-4.0
export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -Wl,-syslibroot,/Developer/SDKs/MacOSX10.4u.sdk -arch i386"
export CPPFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386"
export CFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386"
export SDKROOT="/Developer/SDKs/MacOSX10.4u.sdk"
export MACOSX_DEPLOYMENT_TARGET=10.4

./configure --disable-shared --host=i386
if [  $? -ne 0 ]; then return 1; fi


make -e
if [  $? -ne 0 ]; then return 1; fi
mv libjpeg.a libjpeg_i386.a

if [ ! -d /Developer/SDKs/MacOSX10.5.sdk/ ]; then
    lipo -create libjpeg_i386.a libjpeg_ppc.a -output libjpeg.a

    if [  $? -ne 0 ]; then return 1; fi

    export CC="";export CXX=""
    export LDFLAGS=""
    export CPPFLAGS=""
    export SDKROOT=""

    return 0
fi

# Build for x86_64 architecture if OS 10.5 SDK is present
make clean
if [  $? -ne 0 ]; then return 1; fi

##export PATH=/usr/local/bin:$PATH
export CC=/usr/bin/gcc-4.0;export CXX=/usr/bin/g++-4.0
export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.5.sdk -arch x86_64"
export CPPFLAGS="-isysroot /Developer/SDKs/MacOSX10.5.sdk -arch x86_64"
export CFLAGS="-isysroot /Developer/SDKs/MacOSX10.5.sdk -arch x86_64"
export SDKROOT="/Developer/SDKs/MacOSX10.5.sdk"
export MACOSX_DEPLOYMENT_TARGET=10.5

./configure --disable-shared --host=x86_64
if [  $? -ne 0 ]; then return 1; fi


make -e
if [  $? -ne 0 ]; then return 1; fi

mv libjpeg.a libjpeg_x86_64.a


lipo -create libjpeg_i386.a libjpeg_ppc.a libjpeg_x86_64.a -output libjpeg.a

if [  $? -ne 0 ]; then return 1; fi

export CC="";export CXX=""
export LDFLAGS=""
export CPPFLAGS=""
export CFLAGS=""
export SDKROOT=""

return 0
