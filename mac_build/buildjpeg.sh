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
# 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#
# Script to build Macintosh Universal Binary library of jpeg-6b for
# use in building BOINC.
#
# by Charlie Fenton 4/27/06
#
## In Terminal, CD to the jpeg-6b directory.
##     cd [path]/jpeg-6b/
## then run this script:
##    source [ path_to_this_script ] [ -clean ]
#
# the -clean argument will force a full rebuild.
#

if [ "$1" != "-clean" ]; then
  if [ -f libjpeg_ppc.a ] && [ -f libjpeg_i386.a ] && [ -f libjpeg.a ]; then
    echo "jpeg-6b already built"
    return 0
  fi
fi

export PATH=/usr/local/bin:$PATH
export CC=/usr/bin/gcc-3.3;export CXX=/usr/bin/g++-3.3
export LDFLAGS="-arch ppc"
export CPPFLAGS="-arch ppc"
export SDKROOT="/Developer/SDKs/MacOSX10.3.9.sdk"

./configure --disable-shared --host=ppc
if [  $? -ne 0 ]; then exit 1; fi

rm -f libjpeg_ppc.a
rm -f libjpeg_i386.a
rm -f libjpeg.a
make clean

make -e
if [  $? -ne 0 ]; then exit 1; fi
mv -f libjpeg.a libjpeg_ppc.a

make clean
if [  $? -ne 0 ]; then exit 1; fi

export PATH=/usr/local/bin:$PATH
export CC=/usr/bin/gcc-4.0;export CXX=/usr/bin/g++-4.0
export LDFLAGS=""
export CPPFLAGS=""
export SDKROOT="/Developer/SDKs/MacOSX10.4u.sdk"

./configure --disable-shared --host=i386
if [  $? -ne 0 ]; then exit 1; fi

export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386"
export CPPFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386"

make -e
if [  $? -ne 0 ]; then exit 1; fi
mv libjpeg.a libjpeg_i386.a
lipo -create libjpeg_i386.a libjpeg_ppc.a -output libjpeg.a

if [  $? -ne 0 ]; then exit 1; fi

export CC="";export CXX=""
export LDFLAGS=""
export CPPFLAGS=""
export SDKROOT=""

return 0
