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
# Script to build Macintosh example_app using Makefile
#
# by Charlie Fenton 2/16/10
# Updated 10/11/10 for XCode 3.2 and OS 10.6
# Updated 7/12/12 for Xcode 4.3 and later which are not at a fixed address
# Updated 8/3/12 for TrueType fonts
# Updated 11/8/12 to add slide_show
# Updated 4/14/15 to fix build instructions
# Updated 4/30/20 for Xcode 11
# Updated 2/6/23 to build Universal M1 / x86_64 binary
#
## This script requires OS 10.7 or later
#
## If you drag-install Xcode 4.3 or later, you must have opened Xcode
## and clicked the Install button on the dialog which appears to
## complete the Xcode installation before running this script.
#
## First, build the BOINC libraries using boinc/mac_build/BuildMacBOINC.sh
## This file assumes the locations of the needed libraries are those
## resulting from following the instructions found in the file:
##     boinc/mac_build/HowToBuildBOINC_XCode.rtf
##
## In Terminal, CD to the example_app directory.
##     cd [path]/example_app/
## then run this script:
##     source [path]/MakeMacExample.sh
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

rm -fR x86_64

echo
echo "***************************************************"
echo "******* Building 64-bit Intel Application *********"
echo "***************************************************"
echo

export CC="${GCCPATH}";export CXX="${GPPPATH}"
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64"
export VARIANTFLAGS="-isysroot ${SDKPATH} -arch x86_64 -DMAC_OS_X_VERSION_MAX_ALLOWED=101300 -DMAC_OS_X_VERSION_MIN_REQUIRED=101300 -fvisibility=hidden -fvisibility-inlines-hidden"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.13
export PREFIX="${PREFIX}"

make -f Makefile_mac clean
make -f Makefile_mac all

if [  $? -ne 0 ]; then return $?; fi

mkdir x86_64
mv uc2 x86_64/
mv uc2_graphics x86_64/
mv slide_show x86_64/

rm -f uc2.o
rm -f ttfont.o
rm -f uc2_graphics.o
rm -f slide_show.o

echo
echo "***************************************************"
echo "******* Building arm64 Application *********"
echo "***************************************************"
echo

export CC="${GCCPATH}";export CXX="${GPPPATH}"
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,arm64"
export VARIANTFLAGS="-isysroot ${SDKPATH} -arch arm64 -DMAC_OS_X_VERSION_MAX_ALLOWED=101300 -DMAC_OS_X_VERSION_MIN_REQUIRED=101300 -fvisibility=hidden -fvisibility-inlines-hidden"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.13

make -f Makefile_mac clean
make -f Makefile_mac all

if [  $? -ne 0 ]; then return $?; fi

mkdir arm64
mv uc2 arm64/
mv uc2_graphics arm64/
mv slide_show arm64/

rm -f uc2.o
rm -f ttfont.o
rm -f uc2_graphics.o
rm -f slide_show.o

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
