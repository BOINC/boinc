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
# Script to build Macintosh example_app using Makefile
#
# by Charlie Fenton 5/2/08
#
## In Terminal, CD to the example_app directory.
##     cd [path]/example_app/
## then run this script:
##     sh [path]/MakeMacExample.sh [ -clean ]
##
## the -clean argument will force a full rebuild.
#

rm -fR ppc i386 x86_64

if [ ! -d /Developer/SDKs/MacOSX10.3.9.sdk/ ]; then
    echo "ERROR: System 10.3.9 SDK is missing.  For details, see build instructions at"
    echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or http://boinc.berkeley.edu/trac/wiki/MacBuild"
    exit 1
fi

if [ ! -d /Developer/SDKs/MacOSX10.4u.sdk/ ]; then
    echo "ERROR: System 10.4u SDK is missing.  For details, see build instructions at"
    echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or http://boinc.berkeley.edu/trac/wiki/MacBuild"
    exit 1
fi

echo
echo "***************************************************"
echo "********** Building PowerPC Application ***********"
echo "***************************************************"
echo

## PowerPC build for OS 10.3.0 must use GCC-3.3 and MacOSX10.3.9 SDK
export PATH=/usr/local/bin:$PATH
export MACOSX_DEPLOYMENT_TARGET=10.3
export CC=/usr/bin/gcc-3.3;export CXX=/usr/bin/g++-3.3
export LDFLAGS="-Wl,-syslibroot,/Developer/SDKs/MacOSX10.3.9.sdk -arch ppc"
## If your make file passes LDFLAGS directly to ld instead of to gcc, use the following instead:
## export LDFLAGS="-syslibroot /Developer/SDKs/MacOSX10.3.9.sdk -arch ppc"
export VARIANTFLAGS="-arch ppc -D_NONSTD_SOURCE -isystem /Developer/SDKs/MacOSX10.3.9.sdk"

make -f Makefile_mac clean
make -f Makefile_mac all

if [  $? -ne 0 ]; then exit 1; fi

mkdir ppc
mv uc2 ppc/
mv uc2_graphics ppc/

echo
echo "***************************************************"
echo "******* Building 32-bit Intel Application *********"
echo "***************************************************"
echo

## 32-bit Intel build for OS 10.4 must use GCC-4.0 and MacOSX10.4u SDK

export MACOSX_DEPLOYMENT_TARGET=10.4
export CC=/usr/bin/gcc-4.0;export CXX=/usr/bin/g++-4.0
export LDFLAGS="-Wl,-syslibroot,/Developer/SDKs/MacOSX10.4u.sdk -arch i386"
## If your make file passes LDFLAGS directly to ld instead of to gcc, use the following instead:
## export LDFLAGS="-syslibroot /Developer/SDKs/MacOSX10.3.9.sdk -arch i386"
export VARIANTFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -fvisibility=hidden -fvisibility-inlines-hidden"

make -f Makefile_mac clean
make -f Makefile_mac all

if [  $? -ne 0 ]; then exit 1; fi

mkdir i386
mv uc2 i386/
mv uc2_graphics i386/

## 64-bit Intel build for OS 10.5 must use GCC-4.0 and MacOSX10.5 SDK

# Build for x86_64 architecture only if OS 10.5 SDK is present
if [ -d /Developer/SDKs/MacOSX10.5.sdk/ ]; then

    echo
    echo "***************************************************"
    echo "******* Building 64-bit Intel Application *********"
    echo "***************************************************"
    echo

    export MACOSX_DEPLOYMENT_TARGET=10.5
    export CC=/usr/bin/gcc-4.0;export CXX=/usr/bin/g++-4.0
    export LDFLAGS="-Wl,-syslibroot,/Developer/SDKs/MacOSX10.5.sdk -arch x86_64"
    ## If your make file passes LDFLAGS directly to ld instead of to gcc, use the following instead:
    ## export LDFLAGS="-syslibroot /Developer/SDKs/MacOSX10.3.9.sdk -arch x86_64"
    export VARIANTFLAGS="-isysroot /Developer/SDKs/MacOSX10.5.sdk -arch x86_64 -fvisibility=hidden -fvisibility-inlines-hidden"

    make -f Makefile_mac clean
    make -f Makefile_mac all

    if [  $? -ne 0 ]; then exit 1; fi

    mkdir x86_64
    mv uc2 x86_64/
    mv uc2_graphics x86_64/

fi

echo
echo "***************************************************"
echo "**************** Build Succeeded! *****************"
echo "***************************************************"
echo

export CC="";export CXX=""
export LDFLAGS=""
export CPPFLAGS=""
export CFLAGS=""
export SDKROOT=""

exit 0

