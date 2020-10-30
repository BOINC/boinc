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
# Script to build Macintosh 64-bit Intel library of c-ares for
# use in building BOINC.
#
# by Charlie Fenton 7/21/06
# Updated 10/18/11 for OS 10.7 Lion and XCode 4.2
# Updated 6/25/12 for c-ares 1.9.1
# Updated 7/10/12 for Xcode 4.3 and later which are not at a fixed address
# Updated 2/11/14 for c-ares 1.10.0
# Updated 9/2/14 for bulding c-ares as 64-bit binary
# Updated 9/10/16 for bulding c-ares 1.11.0
# Updated 1/25/18 for bulding c-ares 1.13.0 (updated comemnts only)
# Updated 2/22/18 to avoid APIs not available in earlier versions of OS X
# Updated 1/23/19 use libc++ instead of libstdc++ for Xcode 10 compatibility
# Updated 8/22/20 TO build Apple Silicon / arm64 and x86_64 Universal binary
#
## This script requires OS 10.8 or later
#
## After first installing Xcode, you must have opened Xcode and
## clicked the Install button on the dialog which appears to
## complete the Xcode installation before running this script.
#
## Where x.xx.x is the c-ares version number:
## In Terminal, CD to the c-ares-x.xx.x directory.
##     cd [path]/c-ares-x.xx.x/
## then run this script:
##     source [path]/buildc-ares.sh [ -clean ] [--prefix PATH]
##
## the -clean argument will force a full rebuild.
## if --prefix is given as absolute path the library is installed into there
## use -q or --quiet to redirect build output to /dev/null instead of /dev/stdout
##

function patch_ares_config() {
    # Patch ares_config.h to not use clock_gettime(), which is
    # defined in OS 10.12 SDK but was not available before OS 10.12.
    # If building with an older SDK, this patch will fail because
    # config has already set our desired value.
    rm -f ares_config.h.orig
    rm -f /tmp/ares_config_h_diff
    cat >> /tmp/ares_config_h_diff << ENDOFFILE
--- ares_config_orig.h    2018-01-25 04:15:37.000000000 -0800
+++ ares_config.h    2018-02-22 01:30:57.000000000 -0800
@@ -74,7 +74,7 @@
 #define HAVE_BOOL_T 1

 /* Define to 1 if you have the clock_gettime function and monotonic timer. */
-#define HAVE_CLOCK_GETTIME_MONOTONIC 1
+/* #undef HAVE_CLOCK_GETTIME_MONOTONIC */

 /* Define to 1 if you have the closesocket function. */
 /* #undef HAVE_CLOSESOCKET */
ENDOFFILE

    patch -bfi /tmp/ares_config_h_diff ares_config.h
##    rm -f /tmp/ares_config_h_diff
##    rm -f ares_config.h.rej
}

doclean=""
stdout_target="/dev/stdout"
lprefix="/tmp/installed-c-ares"
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

GCCPATH=`xcrun -find gcc`
if [ $? -ne 0 ]; then
    echo "ERROR: can't find gcc compiler"
    return 1
fi
GCC_can_build_x86_64="no"
GCC_can_build_arm64="no"
GCC_archs=`lipo -info "${GCCPATH}"`
if [[ "${GCC_archs}" == *"x86_64"* ]]; then GCC_can_build_x86_64="yes"; fi
if [[ "${GCC_archs}" == *"arm64"* ]]; then GCC_can_build_arm64="yes"; fi

if [ "${doclean}" != "yes" ]; then
    if [ -f "${libPath}/libcares.a" ]; then
        alreadyBuilt=1

        if [ $GCC_can_build_x86_64 == "yes" ]; then
            lipo "${libPath}/libcares.a" -verify_arch x86_64
            if [ $? -ne 0 ]; then alreadyBuilt=0; doclean="yes"; fi
        fi
        
        if [ $alreadyBuilt -eq 1 ] && [ $GCC_can_build_arm64 == "yes" ]; then
            lipo "${libPath}/libcares.a" -verify_arch arm64
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

export PATH="${TOOLSPATH1}":"${TOOLSPATH2}":/usr/local/bin:$PATH

SDKPATH=`xcodebuild -version -sdk macosx Path`

if [ -d "${libPath}" ]; then
    rm -f "${libPath}/libcares.a"
    if [ $? -ne 0 ]; then return 1; fi
fi

export CC="${GCCPATH}";export CXX="${GPPPATH}"
export CPPFLAGS=""
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64"
export CXXFLAGS="-isysroot ${SDKPATH} -arch x86_64 -stdlib=libc++"
export CFLAGS="-isysroot ${SDKPATH} -arch x86_64"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.7
export MAC_OS_X_VERSION_MAX_ALLOWED=1070
export MAC_OS_X_VERSION_MIN_REQUIRED=1070

./configure --prefix=${lprefix} --enable-shared=NO --host=x86_64
if [ $? -ne 0 ]; then return 1; fi

patch_ares_config

if [ "${doclean}" == "yes" ]; then
    make clean
fi

make 1>$stdout_target
if [ $? -ne 0 ]; then return 1; fi
make install 1>$stdout_target
if [ $? -ne 0 ]; then return 1; fi

# Now see if we can build for arm64
# Note: Some versions of Xcode 12 don't support building for arm64
if [ $GCC_can_build_arm64 == "yes" ]; then

    export CC="${GCCPATH}";export CXX="${GPPPATH}"
    export CPPFLAGS=""
    export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,arm64"
    export CXXFLAGS="-isysroot ${SDKPATH} -target arm64-apple-macos10.7 -stdlib=libc++"
    export CFLAGS="-isysroot ${SDKPATH} -target arm64-apple-macos10.7"
    export SDKROOT="${SDKPATH}"
    export MACOSX_DEPLOYMENT_TARGET=10.7
    export MAC_OS_X_VERSION_MAX_ALLOWED=1070
    export MAC_OS_X_VERSION_MIN_REQUIRED=1070

    ./configure --prefix=${lprefix} --enable-shared=NO --host=arm
    if [ $? -ne 0 ]; then
        echo "              ******"
        echo "c-ares: x86_64 build succeeded but could not build for arm64."
        echo "              ******"
    else

    patch_ares_config

        # save x86_64 header and lib for later use
        # c-ares configure creates a different ares_build.h file for each architecture
        # for a sanity check on size of long and socklen_t. But these are  identical
        # for x86_64 and arm64, so this is not currently an issue. 
        ##    cp -f ares_build.h ares_build_x86_64.h
        mv -f .libs/libcares.a libcares_x86_64.a

        # Build for arm64 architecture
        make clean 1>$stdout_target
        
        make 1>$stdout_target
        if [ $? -ne 0 ]; then 
            rm -f libcares_x86_64.a
            rm -f ares_build_x86_64.h
            return 1
        fi

        # c-ares configure creates a different ares_build.h file for each architecture
        # for a sanity check on size of long and socklen_t. But these are  identical
        # for x86_64 and arm64, so this is not currently an issue. 
        ##     cp -f ares_build.h ares_build_arm64.h
        mv -f .libs/libcares.a .libs/libcares_arm64.a
        # combine x86_64 and arm libraries
        lipo -create libcares_x86_64.a .libs/libcares_arm64.a -output .libs/libcares.a
        if [ $? -ne 0 ]; then
            rm -f libcares_x86_64.a .libs/libcares_arm64.a
            return 1
         fi
         
        rm -f libcares_x86_64.a .libs/libcares_arm64.a
        
        make install 1>$stdout_target
        if [ $? -ne 0 ]; then return 1; fi
    fi
fi

lprefix=""
export CC="";export CXX=""
export LDFLAGS=""
export CXXFLAGS=""
export CFLAGS=""
export SDKROOT=""

return 0
