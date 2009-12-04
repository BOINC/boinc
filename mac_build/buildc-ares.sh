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
# Script to build Macintosh Universal Binary library of c-ares-1.7.0 for
# use in building BOINC.
#
# by Charlie Fenton 7/21/06
# Updated 12/3/09
#
## In Terminal, CD to the c-ares-1.7.0 directory.
##     cd [path]/c-ares-1.7.0/
## then run this script:
##     source [path]/buildc-ares.sh [ -clean ] [ -gcc33 ]
##
## the -clean argument will force a full rebuild.
##
## the -gcc33 argument will cause the PowerPC build to use gcc-3.3
## otherwise both architectures will be built using gcc_4.0
##
## Use -gcc33 if you need to link with a project application using BOINC 
## libraries built with gcc-3.3 for backward compatibility to OS 10.3.0
##
## Build with gcc-4.0 to link with the BOINC client 
#

AlreadyBuilt=0

if [ "$1" != "-clean" ]; then
    if [ -f .libs/libcares_ppc.a ] && [ -f .libs/libcares_i386.a ] && [ -f .libs/libcares.a ]; then
        AlreadyBuilt=1
    fi
fi
    
 if [ -d /Developer/SDKs/MacOSX10.5.sdk/ ]; then
    # Build for x86_64 architecture if OS 10.5 SDK is present
    if [ ! -f .libs/libcares_x86_64.a ]; then
        AlreadyBuilt=0
    fi
fi

if [ $AlreadyBuilt -ne 0 ]; then
    echo "c-ares-1.7.0 already built"
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

if [ "$1" = "-gcc33" ] || [ "$2" = "-gcc33" ]; then
    usegcc33=1
else
    usegcc33=0
fi

export PATH=/usr/local/bin:$PATH
export SDKROOT="/Developer/SDKs/MacOSX10.3.9.sdk"
export MACOSX_DEPLOYMENT_TARGET=10.3

rm -f .libs/libcares.a
rm -f .libs/libcares_ppc.a
rm -f .libs/libcares_i386.a
rm -f .libs/libcares_x86_64.a

if [ $usegcc33 -ne 0 ]; then

export CC=/usr/bin/gcc-3.3;export CXX=/usr/bin/g++-3.3
export LDFLAGS="-arch ppc -D_NONSTD_SOURCE -isystem /Developer/SDKs/MacOSX10.3.9.sdk -Wl,-syslibroot,/Developer/SDKs/MacOSX10.3.9.sdk"
export CPPFLAGS="-arch ppc -D_NONSTD_SOURCE -isystem /Developer/SDKs/MacOSX10.3.9.sdk"
export CFLAGS="-arch ppc -D_NONSTD_SOURCE -isystem /Developer/SDKs/MacOSX10.3.9.sdk"

# curl configure and make expect a path to _installed_ c-ares-1.7.0
# so set a temporary install path that does not contain spaces.
./configure --enable-shared=NO prefix=/tmp/installed-c-ares --host=ppc
if [  $? -ne 0 ]; then return 1; fi

else

export CC=/usr/bin/gcc-4.0;export CXX=/usr/bin/g++-4.0
export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.3.9.sdk -Wl,-syslibroot,/Developer/SDKs/MacOSX10.3.9.sdk -arch ppc"
export CPPFLAGS="-isysroot /Developer/SDKs/MacOSX10.3.9.sdk -arch ppc"
export CFLAGS="-isysroot /Developer/SDKs/MacOSX10.3.9.sdk -arch ppc"

./configure --enable-shared=NO prefix=/tmp/installed-c-ares --host=ppc
fi

if [  $? -ne 0 ]; then return 1; fi


make clean

make
if [  $? -ne 0 ]; then return 1; fi
# c-ares configure creates a different ares_build.h file for each architecture
cp -f ares_build.h ares_build_ppc.h
mv -f .libs/libcares.a libcares_ppc.a

make clean
if [  $? -ne 0 ]; then return 1; fi

##export PATH=/usr/local/bin:$PATH
export CC=/usr/bin/gcc-4.0;export CXX=/usr/bin/g++-4.0
export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -Wl,-syslibroot,/Developer/SDKs/MacOSX10.4u.sdk -arch i386"
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

if [ ! -d /Developer/SDKs/MacOSX10.5.sdk/ ]; then
    mv -f .libs/libcares.a .libs/libcares_i386.a
    mv -f libcares_ppc.a .libs/
    lipo -create .libs/libcares_i386.a .libs/libcares_ppc.a -output .libs/libcares.a
    if [  $? -ne 0 ]; then return 1; fi
    return 0
fi


# Build for x86_64 architecture if OS 10.5 SDK is present

mv -f .libs/libcares.a libcares_i386.a

make clean
if [  $? -ne 0 ]; then return 1; fi

##export PATH=/usr/local/bin:$PATH
export CC=/usr/bin/gcc-4.0;export CXX=/usr/bin/g++-4.0
export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.5.sdk -Wl,-syslibroot,/Developer/SDKs/MacOSX10.5.sdk -arch x86_64"
export CPPFLAGS="-isysroot /Developer/SDKs/MacOSX10.5.sdk -arch x86_64"
export CFLAGS="-isysroot /Developer/SDKs/MacOSX10.5.sdk -arch x86_64"
export SDKROOT="/Developer/SDKs/MacOSX10.5.sdk"
export MACOSX_DEPLOYMENT_TARGET=10.5

./configure --enable-shared=NO prefix=/tmp/installed-c-ares --host=x86_64
if [  $? -ne 0 ]; then return 1; fi

make
if [  $? -ne 0 ]; then return 1; fi
# c-ares configure creates a different ares_build.h file for each architecture
cp -f ares_build.h ares_build_x86_64.h

export CC="";export CXX=""
export LDFLAGS=""
export CPPFLAGS=""
export CFLAGS=""
export SDKROOT=""

mv -f .libs/libcares.a .libs/libcares_x86_64.a
mv -f libcares_ppc.a .libs/
mv -f libcares_i386.a .libs/
lipo -create .libs/libcares_i386.a .libs/libcares_x86_64.a .libs/libcares_ppc.a -output .libs/libcares.a
if [  $? -ne 0 ]; then return 1; fi

return 0
