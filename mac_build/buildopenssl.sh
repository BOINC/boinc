#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2020 University of California
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
# Script to build Macintosh 64-bit Intel openssl libraries
# libcrypto.a and libssl.a for use in building BOINC.
#
# by Charlie Fenton 6/25/12
# Updated 7/10/12 for Xcode 4.3 and later which are not at a fixed address
# Updated 7/30/13 for openssl-1.0.1e
# Updated 2/12/14 for openssl-1.0.1f
# Updated 4/14/14 for openssl-1.0.1g
# Updated 6/6/14 for openssl-1.0.1h
# Updated 9/2/14 for bulding openssl as 64-bit binary
# Updated 6/6/14 for openssl-1.0.1j
# Updated 12/11/15 for openssl-1.0.2e
# Updated 3/2/16 for openssl-1.0.2g
# Updated 9/10/16 for openssl-1.1.0
# Updated 1/25/18 for bulding openssl 1.1.0g (updated comemnts only)
# Updated 1/23/19 use libc++ instead of libstdc++ for Xcode 10 compatibility
# Updated 7/28/20 TO build Apple Silicon / arm64 and x86_64 Universal binary
#
## This script requires OS 10.8 or later
#
## After first installing Xcode, you must have opened Xcode and
## clicked the Install button on the dialog which appears to
## complete the Xcode installation before running this script.
#
## Where x.xx.xy is the openssl version number:
## In Terminal, CD to the openssl-x.xx.xy directory.
##     cd [path]/openssl-x.xx.xy/
## then run this script:
##     source [path]/buildopenssl.sh [ -clean ] [--prefix PATH]
##
## the -clean argument will force a full rebuild.
## if --prefix is given as absolute path the library is installed into there
## use -q or --quiet to redirect build output to /dev/null instead of /dev/stdout
##

doclean=""
stdout_target="/dev/stdout"
lprefix=""
libPath="."
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

export PATH=/usr/local/bin:$PATH

GCCPATH=`xcrun -find gcc`
if [ $? -ne 0 ]; then
    echo "ERROR: can't find gcc compiler"
    return 1
fi

if [ "${doclean}" != "yes" ]; then
    if [ -f ${libPath}/libssl.a ] && [ -f ${libPath}/libcrypto.a ]; then
        alreadyBuilt=1
        GCC_can_build_x86_64="no"
        GCC_can_build_arm64="no"

        GCC_archs=`lipo -archs "${GCCPATH}"`
        if [[ "${GCC_archs}" == *"x86_64"* ]]; then GCC_can_build_x86_64="yes"; fi
        if [[ "${GCC_archs}" == *"arm64"* ]]; then GCC_can_build_arm64="yes"; fi
        if [ $GCC_can_build_x86_64 == "yes" ]; then
            lipo "${libPath}/libssl.a" -verify_arch x86_64
            if [ $? -ne 0 ]; then alreadyBuilt=0; doclean="yes"; fi
            lipo "${libPath}/libcrypto.a" -verify_arch x86_64
            if [ $? -ne 0 ]; then alreadyBuilt=0; doclean="yes"; fi
        fi
        
        if [ $alreadyBuilt -eq 1 ] && [ $GCC_can_build_arm64 == "yes" ]; then
            lipo "${libPath}/libssl.a" -verify_arch arm64
            if [ $? -ne 0 ]; then alreadyBuilt=0; doclean="yes"; fi
            lipo "${libPath}/libcrypto.a" -verify_arch arm64
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

SDKPATH=`xcodebuild -version -sdk macosx Path`

export PATH="${TOOLSPATH1}":"${TOOLSPATH2}":/usr/local/bin:$PATH

if [ -d "${libPath}" ]; then
    rm -f ${libPath}/libssl.a
    rm -f ${libPath}/libcrypto.a
fi

# Build for x86_64 architecture

export CC="${GCCPATH}";export CXX="${GPPPATH}"
export CPPFLAGS=""
export LDFLAGS="-Wl,-sysroot,${SDKPATH},-syslibroot,${SDKPATH},-arch,x86_64"
export CXXFLAGS="-isysroot ${SDKPATH} -arch x86_64 -stdlib=libc++ -DMAC_OS_X_VERSION_MAX_ALLOWED=1070 -DMAC_OS_X_VERSION_MIN_REQUIRED=1070"
export CFLAGS="-isysroot ${SDKPATH} -arch x86_64 -DMAC_OS_X_VERSION_MAX_ALLOWED=1070 -DMAC_OS_X_VERSION_MIN_REQUIRED=1070"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.7
export LIBRARY_PATH="${SDKPATH}/usr/lib"

if [ "x${lprefix}" != "x" ]; then
    ./configure --prefix=${lprefix} no-shared darwin64-x86_64-cc
    if [ $? -ne 0 ]; then return 1; fi
else
    ./configure no-shared darwin64-x86_64-cc
    if [ $? -ne 0 ]; then return 1; fi
fi

if [ "${doclean}" == "yes" ]; then
    make clean 1>$stdout_target
fi

make 1>$stdout_target
if [ $? -ne 0 ]; then return 1; fi

if [ "x${lprefix}" != "x" ]; then
    make install 1>$stdout_target
    if [ $? -ne 0 ]; then return 1; fi
fi

# Try building for arm64 architecture
# Note: Some versions of Xcode 12 don't support building for arm64

export CC="${GCCPATH}";export CXX="${GPPPATH}"
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,arm64"
export CPPFLAGS="-isysroot ${SDKPATH} -target arm64-apple-macos10.7 -DMAC_OS_X_VERSION_MAX_ALLOWED=1070 -DMAC_OS_X_VERSION_MIN_REQUIRED=1070"
export CXXFLAGS="-isysroot ${SDKPATH} -target arm64-apple-macos10.7 -stdlib=libc++ -DMAC_OS_X_VERSION_MAX_ALLOWED=1070 -DMAC_OS_X_VERSION_MIN_REQUIRED=1070"
export CFLAGS="-isysroot ${SDKPATH} -target arm64-apple-macos10.7 -DMAC_OS_X_VERSION_MAX_ALLOWED=1070 -DMAC_OS_X_VERSION_MIN_REQUIRED=1070"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.7

## TEMPORARY WORKAROUNDS because OpenSSL 1.1.0g has no configure option for 
## darwin arm64
if [ "x${lprefix}" != "x" ]; then
    ./configure --prefix=${lprefix} no-shared ios64-cross
    if [ $? -ne 0 ]; then return 1; fi
else
    ./configure no-shared ios64-cross
    if [ $? -ne 0 ]; then return 1; fi
fi
## Patch the ios64-cross makefile to build for a Mac instead of iOS device
sed -i "" 's:-arch arm64:-target arm64-apple-macos10.7 -stdlib=libc++:g' Makefile
sed -i "" 's:-mios-version-min=7.0.0:-DMAC_OS_X_VERSION_MAX_ALLOWED=1070:g' Makefile
sed -i "" 's:-isysroot $(CROSS_TOP)/SDKs/$(CROSS_SDK):-isysroot ${SDKROOT}:g' Makefile
## END OF TEMPORARY WORKAROUNDS

# save x86_64 lib for later use
mv -f libcrypto.a libcrypto_x86_64.a
mv -f libssl.a libssl_x86_64.a

make clean 1>$stdout_target

make 1>$stdout_target
if [ $? -ne 0 ]; then
    echo "              ******"
    echo "OpenSSL: x86_64 build succeeded but could not build for arm64."
    echo "              ******"
    rm -f libcrypto_x86_64.a
    rm -f libssl_x86_64.a
fi

mv -f libcrypto.a libcrypto_arm64.a
mv -f libssl.a libssl_arm64.a

# combine x86_64 and arm libraries
lipo -create libcrypto_x86_64.a libcrypto_arm64.a -output libcrypto.a
if [ $? -eq 0 ]; then
    lipo -create libssl_x86_64.a libssl_arm64.a -output libssl.a
    if [ $? -ne 0 ]; then
        rm -f libcrypto_x86_64.a libcrypto_arm64.a
        rm -f libssl_x86_64.a libssl_arm64.a
        return 1;
    fi
fi

rm -f libcrypto_x86_64.a libcrypto_arm64.a
rm -f libssl_x86_64.a libssl_arm64.a

## openssl 1.1.0g does not have a configure option for darwin arm64, so we use 
## the same configure as for ios64-cross with some patches and hope it works
## properly.
## NOTE: At the time of writing, I do not have an arm64 Mac to test with.
# Revisit this if a newer version of openssl becomes available.
#
# Get the names of the current versions of and openssl from the
# dependencyNames.sh file in the same directory as this script.
myScriptPath="${BASH_SOURCE[0]}"
myScriptDir="${myScriptPath%/*}"
source "${myScriptDir}/dependencyNames.sh"

if [ "${opensslDirName}" != "openssl-1.1.0g" ]; then
echo "${opensslDirName}"
    echo "************ NOTICE ****************"
    echo "New version of openssl may have better arm64 darwin support"
    echo "See comments in build script buildopenssl.sh for details."
    echo "************************************"
    return 
fi

lprefix=""
export CC="";export CXX=""
export LDFLAGS=""
export CXXFLAGS=""
export CFLAGS=""
export SDKROOT=""

return 0
