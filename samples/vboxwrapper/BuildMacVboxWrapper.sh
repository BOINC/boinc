#!/bin/sh

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
# Script to build Macintosh vboxwrapper using Makefile
#
# by Charlie Fenton 2/15/10
# Updated 11/16/11 for XCode 4.1 and OS 10.7
# Updated 7/12/12 for Xcode 4.3 and later which are not at a fixed address
# Updated 4/14/15 for compatibility with Xcode 6
# Updated 6/9/22 for Xcode 13 compatibility (no 32-bit build); add code signing
# Updated 2/6/23 to build Universal M1 / x86_64 binary
#
## This script requires OS 10.15 or later
#
## If you drag-install Xcode 4.3 or later, you must have opened Xcode
## and clicked the Install button on the dialog which appears to
## complete the Xcode installation before running this script.
#
## If you wish to code sign the vboxwrapper, create a file
## ~/BOINCCodeSignIdentities.txt whose first line is your application
## code signing identity.
## First, build the BOINC libraries using boinc/mac_build/BuildMacBOINC.sh
##
## In Terminal, CD to the wrqpper directory.
##     cd [path]/vboxwrapper/
## then run this script:
##     source [path]/BuildMacVboxWrapper.sh
##

# checks if a given path is canonical (absolute and does not contain relative links)
# from http://unix.stackexchange.com/a/256437
isPathCanonical() {
  case "x$1" in
    (x*/..|x*/../*|x../*|x*/.|x*/./*|x./*)
        rc=1
        ;;
    (x/*)
        rc=0
        ;;
    (*)
        rc=1
        ;;
  esac
  return $rc
}

while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        --cache_dir)
        cache_dir="$2"
        shift
        ;;
        *)
        echo "unrecognized option $key"
        ;;
    esac
    shift # past argument or value
done

PREFIX=""
if [ "x$cache_dir" != "x" ]; then
    if isPathCanonical "$cache_dir" && [ "$cache_dir" != "/" ]; then
        PREFIX="$cache_dir"
    else
        echo "cache_dir must be an absolute path without ./ or ../ in it"
        return 1
    fi
fi

GCCPATH=`xcrun -find gcc`
if [  $? -ne 0 ]; then
    echo "ERROR: can't find gcc compiler"
    return $?
fi

GPPPATH=`xcrun -find g++`
if [  $? -ne 0 ]; then
    echo "ERROR: can't find g++ compiler"
    return $?
fi

MAKEPATH=`xcrun -find make`
if [  $? -ne 0 ]; then
    echo "ERROR: can't find make tool"
    return $?
fi

TOOLSPATH1=${MAKEPATH%/make}

ARPATH=`xcrun -find ar`
if [  $? -ne 0 ]; then
    echo "ERROR: can't find ar tool"
    return $?
fi

TOOLSPATH2=${ARPATH%/ar}

export PATH="${TOOLSPATH1}":"${TOOLSPATH2}":/usr/local/bin:$PATH

SDKPATH=`xcodebuild -version -sdk macosx Path`

rm -fR i386 x86_64

echo
echo "***************************************************"
echo "******* Building 64-bit Intel Application *********"
echo "***************************************************"
echo

export CC="${GCCPATH}";export CXX="${GPPPATH}"
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64"
export VARIANTFLAGS="-isysroot ${SDKPATH} -arch x86_64 -DMAC_OS_X_VERSION_MAX_ALLOWED=101300 -DMAC_OS_X_VERSION_MIN_REQUIRED=101300  -stdlib=libc++ -fvisibility=hidden -fvisibility-inlines-hidden"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.13
export PREFIX="${PREFIX}"

make -f Makefile_mac clean
make -f Makefile_mac all

if [  $? -ne 0 ]; then return $?; fi

mv -f vboxwrapper vboxwrapper_x86_64

rm -f vboxwrapper.o

echo
echo "***************************************************"
echo "******* Building arm64 Application *********"
echo "***************************************************"
echo

export CC="${GCCPATH}";export CXX="${GPPPATH}"
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,arm64"
export VARIANTFLAGS="-isysroot ${SDKPATH} -arch arm64 -DMAC_OS_X_VERSION_MAX_ALLOWED=101300 -DMAC_OS_X_VERSION_MIN_REQUIRED=101300 -stdlib=libc++ -fvisibility=hidden -fvisibility-inlines-hidden"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.13

make -f Makefile_mac clean
make -f Makefile_mac all

if [  $? -ne 0 ]; then return $?; fi

mv -f vboxwrapper vboxwrapper_arm64

rm -f vboxwrapper.o

lipo -create vboxwrapper_x86_64 vboxwrapper_arm64 -output vboxwrapper
if [ $? -ne 0 ]; then
    rm -f vboxwrapper_x86_64
    rm -f vboxwrapper_arm64
    rm -f vboxwrapper
    return $?
fi

# Code Sign the vboxwrapper if we have a signing identity
if [ -e "${HOME}/BOINCCodeSignIdentities.txt" ]; then
    exec 8<"${HOME}/BOINCCodeSignIdentities.txt"
    read APPSIGNINGIDENTITY <&8
    codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" vboxwrapper
fi

echo
echo "***************************************************"
echo "**************** Build Succeeded! *****************"
echo "***************************************************"
echo

export CC="";export CXX=""
export LDFLAGS=""
export CPPFLAGS=""
export CXXFLAGS=""
export CFLAGS=""
export SDKROOT=""

return 0
