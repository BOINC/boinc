#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2019 University of California
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
# Updated 1/23/19 use libc++ instead of libstdc++ for Xcode 10 compatibility
#
## This script requires OS 10.8 or later
#
## After first installing Xcode, you must have opened Xcode and
## clicked the Install button on the dialog which appears to
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

# this directory is only used when no --prefix argument was given
rm -fR "../freetype_install/"

# Build for x86_64 architecture
export CC="${GCCPATH}";export CXX="${GPPPATH}"
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64"
export CPPFLAGS=""
export CXXFLAGS="-isysroot ${SDKPATH} -arch x86_64 -stdlib=libc++ -DMAC_OS_X_VERSION_MAX_ALLOWED=1070 -DMAC_OS_X_VERSION_MIN_REQUIRED=1070"
export CFLAGS="-isysroot ${SDKPATH} -arch x86_64 -DMAC_OS_X_VERSION_MAX_ALLOWED=1070 -DMAC_OS_X_VERSION_MIN_REQUIRED=1070"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.7

./configure --enable-shared=NO --prefix=${lprefix} --without-png --host=x86_64
if [ $? -ne 0 ]; then return 1; fi

if [ "${doclean}" = "yes" ]; then
    make clean
fi

make 1>$stdout_target
if [ $? -ne 0 ]; then return 1; fi

# Building ftgl requires [install-path]/bin/freetype-config
# this installs the modified library
make install 1>$stdout_target
if [ $? -ne 0 ]; then return 1; fi

# remove installed items not needed by ftgl build
# this directory is only used when no --prefix argument was given
rm -fR "../freetype_install/share"
rm -f ../freetype_install/lib/libfreetype.*

lprefix=""
export CC="";export CXX=""
export LDFLAGS=""
export CXXFLAGS=""
export CFLAGS=""
export SDKROOT=""

return 0
