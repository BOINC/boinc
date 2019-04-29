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
# of ftgl-2.1.3~rc5 for use in building BOINC graphics.
# The resulting library is at:
#   [path]/ftgl-2.1.3~rc5/src/.libs/libftgl.a
#
# by Charlie Fenton 7/27/12
# Updated 2/7/14 for OS 10.9
# Updated 2/8/18 to fix linker warning for Xcode 9.2 under OS 10.13
# Updated 1/23/19 use libc++ instead of libstdc++ for Xcode 10 compatibility
#
## This script requires OS 10.8 or later
#
## After first installing Xcode, you must have opened Xcode and
## clicked the Install button on the dialog which appears to
## complete the Xcode installation before running this script.
#
## In Terminal, CD to the ftgl-2.1.3~rc5 directory.
##     cd [path]/ftgl-2.1.3~rc5/
## then run this script:
##     source [path]/buildFTGL.sh [ -clean ] [--prefix PATH]
##
## the -clean argument will force a full rebuild.
## if --prefix is given as absolute path the library is installed into there
## use -q or --quiet to redirect build output to /dev/null instead of /dev/stdout
##

doclean=""
stdout_target="/dev/stdout"
lprefix=""
libPath="src/.libs"
libftpath="`pwd`/../freetype_install/"
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -clean|--clean)
        doclean="yes"
        ;;
        -prefix|--prefix)
        lprefix="$2"
        libPath="${lprefix}/lib"
        libftpath="${lprefix}"
        shift
        ;;
        -q|--quiet)
        stdout_target="/dev/null"
        ;;
    esac
    shift # past argument or value
done

# needed for ftgl 2.1.3-rc5 to find our freetype 2.9 build not the system one
export PKG_CONFIG_PATH=${libftpath}/lib/pkgconfig:${PKG_CONFIG_PATH}

SRCDIR=$PWD

if [ "${doclean}" != "yes" ]; then
    if [ -f "${libPath}/libftgl.a" ]; then
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

# Build for x86_64 architecture
export CC="${GCCPATH}";export CXX="${GPPPATH}"
export CPPFLAGS=""
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64"
export CXXFLAGS="-isysroot ${SDKPATH} -arch x86_64 -stdlib=libc++ -DMAC_OS_X_VERSION_MAX_ALLOWED=1070 -DMAC_OS_X_VERSION_MIN_REQUIRED=1070"
export CFLAGS="-isysroot ${SDKPATH} -arch x86_64 -DMAC_OS_X_VERSION_MAX_ALLOWED=1070 -DMAC_OS_X_VERSION_MIN_REQUIRED=1070"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.7

if [ "x${lprefix}" != "x" ]; then
    ./configure --prefix="${lprefix}" --enable-shared=NO --disable-freetypetest --with-ft-prefix="${libftpath}" --host=x86_64
else
    ./configure --enable-shared=NO --disable-freetypetest --with-ft-prefix="${libftpath}" --host=x86_64
fi
if [ $? -ne 0 ]; then return 1; fi

if [ "${doclean}" = "yes" ]; then
    make clean 1>$stdout_target
fi

cd src || return 1
make 1>$stdout_target
if [ $? -ne 0 ]; then
    cd "${SRCDIR}" || return 1
    return 1;
fi

if [ "x${lprefix}" != "x" ]; then
    # this installs the modified library
    make install 1>$stdout_target
    if [ $? -ne 0 ]; then
        cd "${SRCDIR}" || return 1
        return 1;
    fi
fi

cd "${SRCDIR}" || return 1

lprefix=""
export CC="";export CXX=""
export LDFLAGS=""
export CXXFLAGS=""
export CFLAGS=""
export SDKROOT=""

return 0
