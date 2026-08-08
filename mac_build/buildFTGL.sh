#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2023 University of California
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
# Updated 8/22/20 to build Apple Silicon / arm64 and x86_64 Universal binary
# Updated 5/18/21 for compatibility with zsh
# Updated 10/18/21 for building with freetype 2.11.0
# Updated 2/6/23 changed MAC_OS_X_VERSION_MAX_ALLOWED to 101300 and MAC_OS_X_VERSION_MIN_REQUIRED to 101300 and MACOSX_DEPLOYMENT_TARGET to 10.13
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

# needed for ftgl to find our freetype build not the system one
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

GCC_can_build_x86_64="no"
GCC_can_build_arm64="no"
GCC_archs=`lipo -info "${GCCPATH}"`
if [[ "${GCC_archs}" = *"x86_64"* ]]; then GCC_can_build_x86_64="yes"; fi
if [[ "${GCC_archs}" = *"arm64"* ]]; then GCC_can_build_arm64="yes"; fi

if [ "${doclean}" != "yes" ]; then
    if [ -f "${libPath}/libftgl.a" ]; then
        alreadyBuilt=1
        if [ $GCC_can_build_x86_64 = "yes" ]; then
            lipo "${libPath}/libftgl.a" -verify_arch x86_64
            if [ $? -ne 0 ]; then alreadyBuilt=0; doclean="yes"; fi
        fi

        if [ $alreadyBuilt -eq 1 ] && [ $GCC_can_build_arm64 = "yes" ]; then
            lipo "${libPath}/libftgl.a" -verify_arch arm64
            if [ $? -ne 0 ]; then alreadyBuilt=0; doclean="yes"; fi
        fi

        if [ $alreadyBuilt -eq 1 ]; then
            cwd=$(pwd)
            dirname=${cwd##*/}
            echo "${dirname} already built"
            return 0
        fi
    fi
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

## The "-Werror=unguarded-availability" compiler flag generates an error if
## there is an unguarded API not available in our Deployment Target. This
## helps ensure FTGL won't try to use unavailable APIs on older Mac
## systems supported by BOINC.
## It also causes configure to reject any such APIs for which it tests.
##
export CC="${GCCPATH}";export CXX="${GPPPATH}"
export CPPFLAGS=""
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64"
export CXXFLAGS="-isysroot ${SDKPATH} -Werror=unguarded-availability -arch x86_64 -mmacosx-version-min=10.10 -stdlib=libc++ -DMAC_OS_X_VERSION_MAX_ALLOWED=101300 -DMAC_OS_X_VERSION_MIN_REQUIRED=101300"
export CFLAGS="-isysroot ${SDKPATH} -Werror=unguarded-availability -arch x86_64 -mmacosx-version-min=10.10 -DMAC_OS_X_VERSION_MAX_ALLOWED=101300 -DMAC_OS_X_VERSION_MIN_REQUIRED=101300"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.13

if [ "x${lprefix}" != "x" ]; then
    ./configure --prefix="${lprefix}" --enable-shared=NO --disable-freetypetest --with-ft-prefix="${libftpath}" --host=x86_64
else
    ./configure --enable-shared=NO --disable-freetypetest --with-ft-prefix="${libftpath}" --host=x86_64
fi
if [ $? -ne 0 ]; then return 1; fi

if [ "${doclean}" = "yes" ]; then
    make clean 1>$stdout_target
fi

cd src
if [ $? -ne 0 ]; then
    cd "${SRCDIR}"
    return 1
fi

make 1>$stdout_target
if [ $? -ne 0 ]; then
    cd "${SRCDIR}"
    return 1;
fi

cd "${SRCDIR}" || return 1

# Now see if we can build for arm64
# Note: Some versions of Xcode 12 don't support building for arm64
if [ $GCC_can_build_arm64 = "yes" ]; then

    export CC="${GCCPATH}";export CXX="${GPPPATH}"
    export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,arm64"
    export CPPFLAGS="-isysroot ${SDKPATH} -Werror=unguarded-availability -target arm64-apple-macos -mmacosx-version-min=10.10 -stdlib=libc++ -DMAC_OS_X_VERSION_MAX_ALLOWED=101300 -DMAC_OS_X_VERSION_MIN_REQUIRED=101000"
    export CXXFLAGS="-isysroot ${SDKPATH} -Werror=unguarded-availability -target arm64-apple-macos -mmacosx-version-min=10.10 -stdlib=libc++ -DMAC_OS_X_VERSION_MAX_ALLOWED=101300 -DMAC_OS_X_VERSION_MIN_REQUIRED=101000"
    export CFLAGS="-isysroot ${SDKPATH} -Werror=unguarded-availability -mmacosx-version-min=10.10 -target arm64-apple-macos -DMAC_OS_X_VERSION_MAX_ALLOWED=101300 -DMAC_OS_X_VERSION_MIN_REQUIRED=101000"
    export SDKROOT="${SDKPATH}"
    export MACOSX_DEPLOYMENT_TARGET=10.13

    if [ "x${lprefix}" != "x" ]; then
        ./configure --prefix="${lprefix}" --enable-shared=NO --disable-freetypetest --with-ft-prefix="${libftpath}" --host=arm
    else
        ./configure --enable-shared=NO --disable-freetypetest --with-ft-prefix="${libftpath}" --host=arm
    fi
    if [ $? -ne 0 ]; then
        echo "              ******"
        echo "FTGL: x86_64 build succeeded but could not build for arm64."
        echo "              ******"
    else

        # save x86_64 lib for later use
        cd src || return 1
        mv -f .libs/libftgl.a libftgl_x86_64.a
        cd "${SRCDIR}" || return 1

        make clean 1>$stdout_target

        cd src || return 1
        make 1>$stdout_target
        if [ $? -ne 0 ]; then
            rm -f libftgl_x86_64.a
            cd "${SRCDIR}"
            return 1
        fi

        mv -f .libs/libftgl.a .libs/libftgl_arm64.a
        # combine x86_64 and arm libraries
        lipo -create libftgl_x86_64.a .libs/libftgl_arm64.a -output .libs/libftgl.a
        if [ $? -ne 0 ]; then
            rm -f libftgl_x86_64.a libs/libftgl_arm64.a
            cd "${SRCDIR}"
            return 1
        fi

        rm -f libftgl_x86_64.a
        rm -f .libs/libftgl_arm64.a

        cd "${SRCDIR}" || return 1
    fi
fi

if [ "x${lprefix}" != "x" ]; then
    # this installs the modified library
    cd src || return 1
    make install 1>$stdout_target
    if [ $? -ne 0 ]; then
        cd "${SRCDIR}"
        return 1
    fi
fi

cd "${SRCDIR}"

lprefix=""
export CC="";export CXX=""
export LDFLAGS=""
export CXXFLAGS=""
export CFLAGS=""
export SDKROOT=""

return 0
