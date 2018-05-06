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
# of ftgl-2.1.3~rc5 for use in building BOINC graphics.
# The resulting library is at:
#   [path]/ftgl-2.1.3~rc5/src/.libs/libftgl.a
#
# by Charlie Fenton 7/27/12
# Updated 2/7/14 for OS 10.9
# Updated 2/8/18 to fix linker warning for Xcode 9.2 under OS 10.13
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

# Build for i386 architecture
export CC="${GCCPATH}";export CXX="${GPPPATH}"
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,i386"
export CPPFLAGS="-isysroot ${SDKPATH} -arch i386 -DMAC_OS_X_VERSION_MAX_ALLOWED=1060 -DMAC_OS_X_VERSION_MIN_REQUIRED=1060"
export CFLAGS="-isysroot ${SDKPATH} -arch i386 -DMAC_OS_X_VERSION_MAX_ALLOWED=1060 -DMAC_OS_X_VERSION_MIN_REQUIRED=1060"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.6

if [ "x${lprefix}" != "x" ]; then
    ./configure --prefix="${lprefix}" --enable-shared=NO --disable-freetypetest --with-ft-prefix="${libftpath}" --host=i386
    if [ $? -ne 0 ]; then return 1; fi
else
    ./configure --enable-shared=NO --disable-freetypetest --with-ft-prefix="${libftpath}" --host=i386
    if [ $? -ne 0 ]; then return 1; fi
fi

if [ "${doclean}" = "yes" ]; then
    make clean 1>$stdout_target
fi

cd src || return 1
make 1>$stdout_target
if [ $? -ne 0 ]; then
    cd "${SRCDIR}" || return 1
    return 1;
fi

# save i386 lib for later use
mv -f .libs/libftgl.a libftgl_i386.a
cd "${SRCDIR}" || return 1

# Build for x86_64 architecture
make clean 1>$stdout_target

export CC="${GCCPATH}";export CXX="${GPPPATH}"
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64"
export CPPFLAGS="-isysroot ${SDKPATH} -arch x86_64 -DMAC_OS_X_VERSION_MAX_ALLOWED=1060 -DMAC_OS_X_VERSION_MIN_REQUIRED=1060"
export CFLAGS="-isysroot ${SDKPATH} -arch x86_64 -DMAC_OS_X_VERSION_MAX_ALLOWED=1060 -DMAC_OS_X_VERSION_MIN_REQUIRED=1060"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.6

retval=0
if [ "x${lprefix}" != "x" ]; then
    ./configure --prefix="${lprefix}" --enable-shared=NO --disable-freetypetest --with-ft-prefix="${libftpath}" --host=x86_64
    retval=$?
else
    ./configure --enable-shared=NO --disable-freetypetest --with-ft-prefix="${libftpath}" --host=x86_64
    retval=$?
fi
if [ $retval -ne 0 ]; then
    rm -f src/libftgl_i386.a
    return 1;
fi

cd src || return 1
make 1>$stdout_target
if [ $? -ne 0 ]; then
    rm -f libftgl_i386.a
    cd "${SRCDIR}" || return 1
    return 1;
fi

mv -f .libs/libftgl.a .libs/libftgl_x86_64.a
# combine i386 and x86_64 libraries
lipo -create libftgl_i386.a .libs/libftgl_x86_64.a -output .libs/libftgl.a
if [ $? -ne 0 ]; then
    rm -f .libs/libftgl_x86_64.a libftgl_i386.a
    cd "${SRCDIR}" || return 1
    return 1;
fi

rm -f libftgl_i386.a
rm -f .libs/libftgl_x86_64.a

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
export CPPFLAGS=""
export CFLAGS=""
export SDKROOT=""

return 0
