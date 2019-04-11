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
# Script to build Macintosh 64-bit Intel library of sqlite 3 for
# use in building BOINC Manager.
#
# by Charlie Fenton 12/11/12
# Updated 2/11/14 for sqlite 3.8.3
# Updated 1/5/16 for sqlite 3.9.2
# Updated 3/2/16 for sqlite 3.11.0
# Updated 10/22/17 to build 64-bit library (temporarily build both 32-bit and 64-bit libraries)
# Updated 1/25/18 to build only 64-bit library
# Updated 1/23/19 use libc++ instead of libstdc++ for Xcode 10 compatibility
#
## This script requires OS 10.8 or later
#
## After first installing Xcode, you must have opened Xcode and
## clicked the Install button on the dialog which appears to
## complete the Xcode installation before running this script.
#
## Where xxxxxxx is the version string in the directory name:
## In Terminal, CD to the sqlite-autoconf-xxxxxxx directory.
##     cd [path]/sqlite-autoconf-xxxxxxx/
## then run this script:
##     source [path]/buildsqlite3.sh [ -clean ] [--prefix PATH]
##
## the -clean argument will force a full rebuild.
## if --prefix is given as absolute path the library is installed into there
## use -q or --quiet to redirect build output to /dev/null instead of /dev/stdout
##

doclean=""
stdout_target="/dev/stdout"
lprefix=""
libPath=".libs"
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
    if [ -f "${libPath}/libsqlite3.a" ]; then
        lipo "${libPath}/libsqlite3.a" -verify_arch x86_64
        if [ $? -eq 0 ]; then
            cwd=$(pwd)
            dirname=${cwd##*/}
            echo "${dirname} already built"
            return 0
        else
            # already built but not for correct architectures
            doclean="yes"
        fi
    fi
fi

export PATH=/usr/local/bin:$PATH

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

export PATH="${TOOLSPATH1}":"${TOOLSPATH2}":/usr/local/bin:$PATH

SDKPATH=`xcodebuild -version -sdk macosx Path`

if [ -d "${libPath}" ]; then
    rm -f "${libPath}/libsqlite3.a"
fi

# Build for x86_64 architecture
export PATH=/usr/local/bin:$PATH
export CC="${GCCPATH}";export CXX="${GPPPATH}"
export CPPFLAGS=""
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64"
export CXXFLAGS="-Os -isysroot ${SDKPATH} -arch x86_64 -stdlib=libc++ -DMAC_OS_X_VERSION_MAX_ALLOWED=1070 -DMAC_OS_X_VERSION_MIN_REQUIRED=1070"
export CFLAGS="-Os -isysroot ${SDKPATH} -arch x86_64 -DMAC_OS_X_VERSION_MAX_ALLOWED=1070 -DMAC_OS_X_VERSION_MIN_REQUIRED=1070"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.7

if [ "x${lprefix}" != "x" ]; then
    ./configure --prefix=${lprefix} --enable-shared=NO --host=x86_64
    if [ $? -ne 0 ]; then return 1; fi
else
    ./configure --enable-shared=NO --host=x86_64
    if [ $? -ne 0 ]; then return 1; fi
fi

if [ "${doclean}" = "yes" ]; then
    make clean 1>$stdout_target
fi

make 1>$stdout_target
if [ $? -ne 0 ]; then return 1; fi

if [ "x${lprefix}" != "x" ]; then
    make install 1>$stdout_target
    if [ $? -ne 0 ]; then return 1; fi
fi

lprefix=""
export CC="";export CXX=""
export LDFLAGS=""
export CXXFLAGS=""
export CFLAGS=""
export SDKROOT=""

return 0
