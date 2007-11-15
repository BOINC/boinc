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
# Script to build Macintosh Universal Binary library of curl-7.17.1 for
# use in building BOINC.
#
# by Charlie Fenton 7/21/06
# Updated for curl-7.17.1 11/15/07
#
## In Terminal, CD to the curl-7.17.1 directory.
##     cd [path]/curl-7.17.1/
## then run this script:
##     source [path]/buildcurl.sh [ -clean ] [ -gcc33 ]
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

DarwinVersion=`uname -r`;
DarwinMajorVersion=`echo $DarwinVersion | sed 's/\([0-9]*\)[.].*/\1/' `;
# DarwinMinorVersion=`echo $version | sed 's/[0-9]*[.]\([0-9]*\).*/\1/' `;
#
# echo "major = $DarwinMajorVersion"
# echo "minor = $DarwinMinorVersion"
#
# Darwin version 9.x.y corresponds to OS 10.5.x
# Darwin version 8.x.y corresponds to OS 10.4.x
# Darwin version 7.x.y corresponds to OS 10.3.x
# Darwin version 6.x corresponds to OS 10.2.x

AlreadyBuilt=0

if [ "$1" != "-clean" ]; then
    if [ -f lib/.libs/libcurl_ppc.a ] && [ -f lib/.libs/libcurl_i386.a ] && [ -f lib/.libs/libcurl.a ]; then
        AlreadyBuilt=1
    fi
fi
    
 if [ "$DarwinMajorVersion" = "9" ]; then
    # OS 10.5
    if [ ! -f lib/.libs/libcurl_x86_64.a ]; then
        AlreadyBuilt=0
    fi
fi

   
if [ $AlreadyBuilt -ne 0 ]; then
    echo "curl-7.17.1 already built"
    return 0
fi

if [ "$1" = "-gcc33" ] || [ "$2" = "-gcc33" ]; then
    usegcc33=1
else
    usegcc33=0
fi

export PATH=/usr/local/bin:$PATH
export LDFLAGS="-arch ppc"
export CFLAGS="-arch ppc"
export SDKROOT="/Developer/SDKs/MacOSX10.3.9.sdk"

rm -fR macfix

if [ $usegcc33 -ne 0 ]; then

export CC=/usr/bin/gcc-3.3;export CXX=/usr/bin/g++-3.3

## ./configure --enable-shared=NO --host=ppc
./configure --enable-shared=NO --host=ppc CPPFLAGS="-arch ppc -I/Developer/SDKs/MacOSX10.3.9.sdk/Developer/Headers/FlatCarbon -I/Developer/SDKs/MacOSX10.3.9.sdk/usr/include -isystem /Developer/SDKs/MacOSX10.3.9.sdk/usr/include/gcc/darwin/3.3 -I/Developer/SDKs/MacOSX10.3.9.sdk/usr/include/gcc/darwin/3.3/c++ -I/Developer/SDKs/MacOSX10.3.9.sdk/usr/include/gcc/darwin/3.3/c++/ppc-darwin -isystem /Developer/SDKs/MacOSX10.3.9.sdk/usr/include"
if [  $? -ne 0 ]; then return 1; fi

else

## workaround for header problem
mkdir macfix
echo "#include <gcc/darwin/3.3/stdarg.h>" >> ./macfix/stdarg.h

export CC=/usr/bin/gcc-4.0;export CXX=/usr/bin/g++-4.0

## ./configure --enable-shared=NO --host=ppc
./configure --enable-shared=NO --host=ppc CPPFLAGS="-arch ppc -I${PWD}/macfix -I/Developer/SDKs/MacOSX10.3.9.sdk/Developer/Headers/FlatCarbon -I/Developer/SDKs/MacOSX10.3.9.sdk/usr/include"
fi

if [  $? -ne 0 ]; then return 1; fi


make clean

rm -f lib/.libs/libcurl.a
rm -f lib/.libs/libcurl_ppc.a
rm -f lib/.libs/libcurl_i386.a

export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.3.9.sdk -Wl,-syslibroot,/Developer/SDKs/MacOSX10.3.9.sdk"

make
if [  $? -ne 0 ]; then return 1; fi
mv -f lib/.libs/libcurl.a lib/libcurl_ppc.a

rm -fR macfix

make clean
if [  $? -ne 0 ]; then return 1; fi

export PATH=/usr/local/bin:$PATH
export CC=/usr/bin/gcc-4.0;export CXX=/usr/bin/g++-4.0
export LDFLAGS=""
export CPPFLAGS=""
export CFLAGS=""
export SDKROOT="/Developer/SDKs/MacOSX10.4u.sdk"

## ./configure --enable-shared=NO --host=i386
./configure --enable-shared=NO --host=i386 CPPFLAGS="-arch i386 -I/Developer/SDKs/MacOSX10.4u.sdk/Developer/Headers/FlatCarbon -isystem /Developer/SDKs/MacOSX10.4u.sdk/usr/include"
if [  $? -ne 0 ]; then return 1; fi

export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386"
export CPPFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386"

make
if [  $? -ne 0 ]; then return 1; fi

export CC="";export CXX=""
export LDFLAGS=""
export CPPFLAGS=""
export CFLAGS=""
export SDKROOT=""

if [ "$DarwinMajorVersion" != "9" ]; then
    mv -f lib/.libs/libcurl.a lib/.libs/libcurl_i386.a
    mv -f lib/libcurl_ppc.a lib/.libs/
    lipo -create lib/.libs/libcurl_i386.a lib/.libs/libcurl_ppc.a -output lib/.libs/libcurl.a
    if [  $? -ne 0 ]; then return 1; fi
    return 0
fi


# OS 10.5

mv -f lib/.libs/libcurl.a lib/libcurl_i386.a

make clean
if [  $? -ne 0 ]; then return 1; fi

export PATH=/usr/local/bin:$PATH
export CC=/usr/bin/gcc-4.0;export CXX=/usr/bin/g++-4.0
export LDFLAGS=""
export CPPFLAGS=""
export CFLAGS=""
export SDKROOT="/Developer/SDKs/MacOSX10.5u.sdk"

## ./configure --enable-shared=NO --host=i386
./configure --enable-shared=NO --host=x86_64 --without-random CFLAGS="-arch x86_64" CPPFLAGS="-arch x86_64 -DHAVE_POSIX_STRERROR_R=1 -I/Developer/SDKs/MacOSX10.5.sdk/Developer/Headers/FlatCarbon -isystem /Developer/SDKs/MacOSX10.5.sdk/usr/include"
if [  $? -ne 0 ]; then return 1; fi

export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.5.sdk -arch x86_64"
export CPPFLAGS="-isysroot /Developer/SDKs/MacOSX10.5.sdk -arch x86_64"

make
if [  $? -ne 0 ]; then return 1; fi

export CC="";export CXX=""
export LDFLAGS=""
export CPPFLAGS=""
export CFLAGS=""
export SDKROOT=""

mv -f lib/.libs/libcurl.a lib/.libs/libcurl_x86_64.a
mv -f lib/libcurl_ppc.a lib/.libs/
mv -f lib/libcurl_i386.a lib/.libs/
lipo -create lib/.libs/libcurl_i386.a lib/.libs/libcurl_x86_64.a lib/.libs/libcurl_ppc.a -output lib/.libs/libcurl.a
if [  $? -ne 0 ]; then return 1; fi

return 0
