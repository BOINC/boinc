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
# Script to build Macintosh Universal Intel library (i386 and x86_64) 
# of ftgl-2.1.3~rc5 for use in building BOINC graphics.
# The resulting library is at:
#   [path]/ftgl-2.1.3~rc5/src/.libs/libftgl.a
#
# by Charlie Fenton 7/27/12
#
## This script requires OS 10.6 or later
#
## If you drag-install Xcode 4.3 or later, you must have opened Xcode 
## and clicked the Install button on the dialog which appears to 
## complete the Xcode installation before running this script.
#
## In Terminal, CD to the ftgl-2.1.3~rc5 directory.
##     cd [path]/ftgl-2.1.3~rc5/
## then run this script:
##     source [path]/buildFTGL.sh [ -clean ]
##
## the -clean argument will force a full rebuild.
##

if [ "$1" != "-clean" ]; then
    if [ -f src/.libs/libftgl.a ]; then
        echo "ftgl-2.1.3~rc5 already built"
        return 0
    fi
fi

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

export PATH="${TOOLSPATH1}":"${TOOLSPATH2}":$PATH

SDKPATH=`xcodebuild -version -sdk macosx Path`

if [  $? -ne 0 ]; then return 1; fi

# Build for i386 architecture
export CC="${GCCPATH}";export CXX="${GPPPATH}"
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,i386"
export CPPFLAGS="-isysroot ${SDKPATH} -arch i386 -DMAC_OS_X_VERSION_MAX_ALLOWED=1040 -DMAC_OS_X_VERSION_MIN_REQUIRED=1040"
export CFLAGS="-isysroot ${SDKPATH} -arch i386 -DMAC_OS_X_VERSION_MAX_ALLOWED=1040 -DMAC_OS_X_VERSION_MIN_REQUIRED=1040"
export SDKROOT="${SDKPATH}"

./configure --enable-shared=NO --disable-freetypetest --host=i386
if [  $? -ne 0 ]; then return 1; fi

if [ "$1" = "-clean" ]; then
    make clean
fi

cd src
make
if [  $? -ne 0 ]; then 
    cd ..
    return 1; 
fi

mv -f .libs/libftgl.a libftgl_i386.a
cd ..

# Build for x86_64 architecture
make clean

export CC="${GCCPATH}";export CXX="${GPPPATH}"
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64"
export CPPFLAGS="-isysroot ${SDKPATH} -arch x86_64 -DMAC_OS_X_VERSION_MAX_ALLOWED=1040 -DMAC_OS_X_VERSION_MIN_REQUIRED=1040"
export CFLAGS="-isysroot ${SDKPATH} -arch x86_64 -DMAC_OS_X_VERSION_MAX_ALLOWED=1040 -DMAC_OS_X_VERSION_MIN_REQUIRED=1040"
export SDKROOT="${SDKPATH}"

./configure --enable-shared=NO --disable-freetypetest --host=x86_64
if [  $? -ne 0 ]; then
    rm -f src/libftgl_i386.a
    return 1;
fi

cd src
make
if [  $? -ne 0 ]; then 
    rm -f libftgl_i386.a
    cd ..
    return 1; 
fi

mv -f .libs/libftgl.a .libs/libftgl_x86_64.a

lipo -create libftgl_i386.a .libs/libftgl_x86_64.a -output .libs/libftgl.a

if [  $? -ne 0 ]; then 
    cd ..
    return 1; 
fi

rm -f libftgl_i386.a
rm -f .libs/libftgl_x86_64.a

cd ..

export CC="";export CXX=""
export LDFLAGS=""
export CPPFLAGS=""
export CFLAGS=""
export SDKROOT=""

return 0
