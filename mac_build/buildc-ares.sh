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
# Script to build Macintosh Universal Binary library of c-ares-1.5.1 for
# use in building BOINC.
#
# by Charlie Fenton 7/21/06
# Updated 1/29/08
#
## In Terminal, CD to the c-ares-1.5.1 directory.
##     cd [path]/c-ares-1.5.1/
## then run this script:
##     source [path]/buildc-ares.sh [ -clean ]
##
## the -clean argument will force a full rebuild.
#

if [ "$1" != "-clean" ]; then
    if [ -f .libs/libcares_ppc.a ] && [ -f .libs/libcares_i386.a ] && [ -f .libs/libcares.a ]; then
    echo "c-ares-1.5.1 already built"
    return 0
    fi
fi
    
export PATH=/usr/local/bin:$PATH
export CC=/usr/bin/gcc-3.3;export CXX=/usr/bin/g++-3.3
export LDFLAGS="-arch ppc -D_NONSTD_SOURCE -isystem /Developer/SDKs/MacOSX10.3.9.sdk -Wl,-syslibroot,/Developer/SDKs/MacOSX10.3.9.sdk"
export CPPFLAGS="-arch ppc -D_NONSTD_SOURCE -isystem /Developer/SDKs/MacOSX10.3.9.sdk"
export CFLAGS="-arch ppc -D_NONSTD_SOURCE -isystem /Developer/SDKs/MacOSX10.3.9.sdk"
export SDKROOT="/Developer/SDKs/MacOSX10.3.9.sdk"
export MACOSX_DEPLOYMENT_TARGET=10.3

rm -f .libs/libcares.a
rm -f .libs/libcares_ppc.a
rm -f .libs/libcares_i386.a

./configure --enable-shared=NO --host=ppc
if [  $? -ne 0 ]; then return 1; fi

make clean

make
if [  $? -ne 0 ]; then return 1; fi
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

./configure --enable-shared=NO --host=i386
if [  $? -ne 0 ]; then return 1; fi

make
if [  $? -ne 0 ]; then return 1; fi

export CC="";export CXX=""
export LDFLAGS=""
export CPPFLAGS=""
export CFLAGS=""
export SDKROOT=""

mv -f .libs/libcares.a .libs/libcares_i386.a
mv -f libcares_ppc.a .libs/
lipo -create .libs/libcares_i386.a .libs/libcares_ppc.a -output .libs/libcares.a
if [  $? -ne 0 ]; then return 1; fi

return 0
