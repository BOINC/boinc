#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2017 University of California
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
# of FreeType for use in building BOINC graphics.
# The resulting library is at:
#   [path]/freetype-x.x.x/objs/.libs/libfreetype.a
# where x.x.x is the freetype version number

#
# by Charlie Fenton 7/27/12
# Updated 2/7/14 for OS 10.9
# Updated 4/8/15 to check for spaces in path
# Updated 1/5/16 for FreeType-2.6.2
# Updated 1/25/18 for any version of FreeType (changed only comments)
#
## This script requires OS 10.6 or later
#
## If you drag-install Xcode 4.3 or later, you must have opened Xcode
## and clicked the Install button on the dialog which appears to
## complete the Xcode installation before running this script.
#
## Where x.x.x is the freetype version number:
## In Terminal, CD to the freetype-x.x.x directory.
##     cd [path]/freetype-x.x.x/
## then run this script:
##     source [path]/buildfreetype.sh [ -clean ] [--prefix PATH]
##
## the -clean argument will force a full rebuild.
## if --prefix is given as absolute path the library is installed into there
## use -q or --quiet to redirect build output to /dev/null instead of /dev/stdout
##


SRCDIR=$PWD

echo "${SRCDIR}" | grep " " > /dev/null 2>&1
if [ "$?" -eq "0" ]; then
    echo "**********************************************************"
    echo "**********************************************************"
    echo "**********                                      **********"
    echo "********** ERROR: Path must not contain spaces! **********"
    echo "**********                                      **********"
    echo "**********************************************************"
    echo "**********************************************************"
    echo "**********************************************************"
    return 1
fi

doclean=""
stdout_target="/dev/stdout"
lprefix="`pwd`/../freetype_install/"
libPath="objs/.libs"
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -clean|--clean)
        doclean="yes"
        ;;
        -prefix|--prefix)
        lprefix="$2"
        libPath="${lprefix}/lib"
        shift
        ;;
        -q|--quiet)
        stdout_target="/dev/null"
        ;;
    esac
    shift # past argument or value
done

if [ "${doclean}" != "yes" ]; then
    if [ -f "${libPath}/libfreetype.a" ]; then
        cwd=$(pwd)
        dirname=${cwd##*/}
        echo "${dirname} already built"
        return 0
    fi
fi

GCCPATH=`xcrun -find gcc`
if [ $? -ne 0 ]; then
    echo "ERROR: can't find gcc compiler"
    return 1
fi

GPPPATH=`xcrun -find g++`
if [ $? -ne 0 ]; then
    echo "ERROR: can't find g++ compiler"
    return 1
fi

MAKEPATH=`xcrun -find make`
if [ $? -ne 0 ]; then
    echo "ERROR: can't find make tool"
    return 1
fi

TOOLSPATH1=${MAKEPATH%/make}

ARPATH=`xcrun -find ar`
if [ $? -ne 0 ]; then
    echo "ERROR: can't find ar tool"
    return 1
fi

TOOLSPATH2=${ARPATH%/ar}

export PATH="${TOOLSPATH1}":"${TOOLSPATH2}":$PATH

SDKPATH=`xcodebuild -version -sdk macosx Path`

# Build for i386 architecture
export CC="${GCCPATH}";export CXX="${GPPPATH}"
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,i386"
export CPPFLAGS="-isysroot ${SDKPATH} -arch i386 -DMAC_OS_X_VERSION_MAX_ALLOWED=1060 -DMAC_OS_X_VERSION_MIN_REQUIRED=1060"
export CFLAGS="-isysroot ${SDKPATH} -arch i386 -DMAC_OS_X_VERSION_MAX_ALLOWED=1060 -DMAC_OS_X_VERSION_MIN_REQUIRED=1060"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.6

cp -p objs/README README-objs
rm -fR objs/*.*
rm -fR objs/*
cp -p README-objs objs/README
rm -f README-objs
# this directory is only used when no --prefix argument was given
rm -fR "../freetype_install/"

./configure --enable-shared=NO --prefix=${lprefix} --host=i386
if [ $? -ne 0 ]; then return 1; fi

if [ "${doclean}" = "yes" ]; then
    make clean
fi

make 1>$stdout_target
if [ $? -ne 0 ]; then return 1; fi

# save i386 lib for later use
mv -f objs/.libs/libfreetype.a objs/.libs/libfreetype_i386.a

# Build for x86_64 architecture
make clean 1>$stdout_target

export CC="${GCCPATH}";export CXX="${GPPPATH}"
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64"
export CPPFLAGS="-isysroot ${SDKPATH} -arch x86_64 -DMAC_OS_X_VERSION_MAX_ALLOWED=1060 -DMAC_OS_X_VERSION_MIN_REQUIRED=1060"
export CFLAGS="-isysroot ${SDKPATH} -arch x86_64 -DMAC_OS_X_VERSION_MAX_ALLOWED=1060 -DMAC_OS_X_VERSION_MIN_REQUIRED=1060"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.6

./configure --enable-shared=NO --prefix=${lprefix} --host=x86_64
if [ $? -ne 0 ]; then return 1; fi
make 1>$stdout_target
if [ $? -ne 0 ]; then return 1; fi

mv -f objs/.libs/libfreetype.a objs/.libs/libfreetype_x86_64.a
# combine i386 and x86_64 libraries
lipo -create objs/.libs/libfreetype_i386.a objs/.libs/libfreetype_x86_64.a -output objs/.libs/libfreetype.a
if [ $? -ne 0 ]; then return 1; fi

rm -f objs/.libs/libfreetype_i386.a
rm -f objs/.libs/libfreetype_x86_64.a

# Building ftgl requires [install-path]/bin/freetype-config
# this installs the modified library
make install 1>$stdout_target
if [ $? -ne 0 ]; then return 1; fi

# remove installed items not needed by ftgl build
# this directory is only used when no --prefix argument was given
rm -fR "../freetype_install/share"
rm -fR "../freetype_install/lib"

lprefix=""
export CC="";export CXX=""
export LDFLAGS=""
export CPPFLAGS=""
export CFLAGS=""
export SDKROOT=""

return 0
