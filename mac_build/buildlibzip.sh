#!/bin/bash

# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2025 University of California
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
# Script to build Macintosh 64-bit Intel library of libzip for
# use in building BOINC.
#
#
## This script requires OS 10.8 or later
#
## After first installing Xcode, you must have opened Xcode and
## clicked the Install button on the dialog which appears to
## complete the Xcode installation before running this script.
#
## Where x.xx.x is the libzip version number:
## In Terminal, CD to the libzip-x.xx.x directory.
##     cd [path]/libzip-x.xx.x/
## then run this script:
##     source [path]/buildlibzip.sh [ -clean ] [--prefix PATH]
##
## the -clean argument will force a full rebuild.
## if --prefix is given as absolute path the library is installed into there
## use -q or --quiet to redirect build output to /dev/null instead of /dev/stdout
##
## NOTE: This script requires CMake. Before running this script you must
## install CMake for command line use:
##   Download CMake from https://cmake.org/download/
##   Open the disk image file and drag the CMAKE app into the /Applications
##       directory
##   Eject the disk image and open the Cmake app
##   Enter the following in Terminal:
##       sudo "/Applications/CMake.app/Contents/bin/cmake-gui" --install
##   Emter your password when prompted
##

doclean=""
stdout_target="/dev/stdout"
lprefix="/tmp/installed-libzip"
libPath="./lib"
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
    if [ -f "${libPath}/libzip.a" ]; then
        alreadyBuilt=1

        if [ $GCC_can_build_x86_64 = "yes" ]; then
            lipo "${libPath}/libzip.a" -verify_arch x86_64
            if [ $? -ne 0 ]; then alreadyBuilt=0; doclean="yes"; fi
        fi

        if [ $alreadyBuilt -eq 1 ] && [ $GCC_can_build_arm64 = "yes" ]; then
            lipo "${libPath}/libzip.a" -verify_arch arm64
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

CMAKEPATH=`xcrun -find cmake`
if [ $? -ne 0 ]; then
    echo "ERROR: can't find cmake tool"
    return 1
fi

TOOLSPATH3=${CMAKEPATH%/cmake}

export PATH="${TOOLSPATH1}":"${TOOLSPATH2}":"${TOOLSPATH3}":/usr/local/bin:$PATH

SDKPATH=`xcodebuild -version -sdk macosx Path`

if [ -d "${libPath}" ]; then
    rm -f "${libPath}/libzip.a"
    if [ $? -ne 0 ]; then return 1; fi
fi

# Build for x86_64 architecture

## The "-Werror=unguarded-availability" compiler flag generates an error if
## there is an unguarded API not available in our Deployment Target. This
## helps ensure libzip won't try to use unavailable APIs on older Mac
## systems supported by BOINC.
## It also causes configure to reject any such APIs for which it tests.
#
export CC="${GCCPATH}";export CXX="${GPPPATH}"
export CPPFLAGS=""
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64"
export CXXFLAGS="-isysroot ${SDKPATH} -Werror=unguarded-availability -arch x86_64 -mmacosx-version-min=10.13 -stdlib=libc++"
export CFLAGS="-isysroot ${SDKPATH} -Werror=unguarded-availability -mmacosx-version-min=10.13 -arch x86_64"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.13
export MAC_OS_X_VERSION_MAX_ALLOWED=101300
export MAC_OS_X_VERSION_MIN_REQUIRED=101300

cmake --fresh -B . -S . -DCMAKE_INSTALL_PREFIX=${lprefix} -DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DENABLE_COMMONCRYPTO=OFF -DENABLE_GNUTLS=OFF -DENABLE_MBEDTLS=OFF -DENABLE_OPENSSL=OFF -DENABLE_WINDOWS_CRYPTO=OFF -DENABLE_BZIP2=OFF -DENABLE_LZMA=OFF -DENABLE_ZSTD=OFF -DENABLE_FDOPEN=OFF -DBUILD_TOOLS=OFF -DBUILD_REGRESS=OFF -DBUILD_OSSFUZZ=OFF -DBUILD_EXAMPLES=OFF -DBUILD_DOC=OFF
if [ $? -ne 0 ]; then return 1; fi

if [ "${doclean}" = "yes" ]; then
    make clean
fi

make 1>$stdout_target
if [ $? -ne 0 ]; then return 1; fi
make install 1>$stdout_target
if [ $? -ne 0 ]; then return 1; fi

# Now see if we can build for arm64
# Note: Some versions of Xcode 12 don't support building for arm64
if [ $GCC_can_build_arm64 = "yes" ]; then

    export CC="${GCCPATH}";export CXX="${GPPPATH}"
    export CPPFLAGS=""
    export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,arm64"
    export CXXFLAGS="-isysroot ${SDKPATH} -Werror=unguarded-availability -target arm64-apple-macos10.13 -mmacosx-version-min=10.13 -stdlib=libc++"
    export CFLAGS="-isysroot ${SDKPATH} -Werror=unguarded-availability -mmacosx-version-min=10.13 -target arm64-apple-macos10.13"
    export SDKROOT="${SDKPATH}"
    export MACOSX_DEPLOYMENT_TARGET=10.13
    export MAC_OS_X_VERSION_MAX_ALLOWED=101300
    export MAC_OS_X_VERSION_MIN_REQUIRED=101300

    cmake --fresh -B . -S . -DCMAKE_INSTALL_PREFIX=${lprefix} -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DENABLE_COMMONCRYPTO=OFF -DENABLE_GNUTLS=OFF -DENABLE_MBEDTLS=OFF -DENABLE_OPENSSL=OFF -DENABLE_WINDOWS_CRYPTO=OFF -DENABLE_BZIP2=OFF -DENABLE_LZMA=OFF -DENABLE_ZSTD=OFF -DENABLE_FDOPEN=OFF -DBUILD_TOOLS=OFF -DBUILD_REGRESS=OFF -DBUILD_OSSFUZZ=OFF -DBUILD_EXAMPLES=OFF -DBUILD_DOC=OFF
    if [ $? -ne 0 ]; then
        echo "              ******"
        echo "libzip: x86_64 build succeeded but could not build for arm64."
        echo "              ******"
    else

        mv -f ./lib/libzip.a ./lib/libzip_x86_64.a

        # Build for arm64 architecture
        make clean 1>$stdout_target

        make 1>$stdout_target
        if [ $? -ne 0 ]; then
            rm -f ./lib/libzip_x86_64.a
            return 1
        fi

        mv -f ./lib/libzip.a ./lib/libzip_arm64.a

        # combine x86_64 and arm libraries
        lipo -create ./lib/libzip_x86_64.a ./lib/libzip_arm64.a -output "./lib/libzip.a"
        if [ $? -ne 0 ]; then
            rm -f ./lib/libzip_x86_64.a ./lib/libzip_arm64.a
            return 1
         fi

        rm -f ./lib/libzip_x86_64.a ./lib/libzip_arm64.a
    fi
fi

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
