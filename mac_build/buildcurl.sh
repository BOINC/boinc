#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
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
# Script to build Macintosh 64-bit Intel library of curl for
# use in building BOINC.
#
# by Charlie Fenton 7/21/06
# Updated 12/3/09 for OS 10.7 Lion and XCode 4.2
# Updated 6/25/12 for curl 7.26.0
# Updated 7/10/12 for Xcode 4.3 and later which are not at a fixed address
# Updated 2/11/14 for curl 7.35.0 with c-ares 1.10.0
# Updated 9/2/14 for bulding curl as 64-bit binary
# Updated 11/17/14 for curl 7.39.0 with c-ares 1.10.0
# Updated 12/11/15 for curl 7.46.0 with c-ares 1.10.0
# Updated 3/2/16 for curl 7.47.1 with c-ares 1.10.0
# Updated 9/10/16 for curl 7.50.2 with c-ares 1.11.0
# Updated 3/14/17 to patch curlrules.h to fix BOINC Manager compile error
# Updated 1/25/18 for curl 7.58.0 with c-ares 1.13.0 & openssl 1.1.0g, don't patch currules.h
# Updated 1/26/18 to get directory names of c-ares and OpenSSL from dependencyNames.sh
# Updated 2/22/18 to avoid APIs not available in earlier versions of OS X
# Updated 1/23/19 use libc++ instead of libstdc++ for Xcode 10 compatibility
# Updated 8/22/20 to build Apple Silicon / arm64 and x86_64 Universal binary
# Updated 12/24/20 for curl 7.73.0
# Updated 5/18/21 for compatibility with zsh
# Updated 10/11/21 to use Secure Transport instead of OpenSSL (uses MacOS certificate store
#   instead of ca-bundle.crt)
# Updated 11/16/21 for curl 7.79.1
# Updated 2/6/23 changed MAC_OS_X_VERSION_MAX_ALLOWED to 101300 and MAC_OS_X_VERSION_MIN_REQUIRED to 101300 and MACOSX_DEPLOYMENT_TARGET to 10.13
# Updated 4/5/23 for args now accepted by patch utility; set mmacosx-version-min=10.13
# Updated 10/19/25 for curl 8.16.0. Secure Transport is deprecated so use OpenSSl again
#
## Curl's configure and make set the "-Werror=partial-availability" compiler flag,
## which generates an error if there is an API not available in our Deployment
## Target. This helps ensure curl won't try to use unavailable APIs on older Mac
## systems supported by BOINC.
#
## After first installing Xcode, you must have opened Xcode and
## clicked the Install button on the dialog which appears to
## complete the Xcode installation before running this script.
#
## Where x.xx.x is the curl version number:
## In Terminal, CD to the curl-x.xx.x directory.
##     cd [path]/curl-x.xx.x/
## then run this script:
##     source [path]/buildcurl.sh [ -clean ] [--prefix PATH]
##
## the -clean argument will force a full rebuild.
## if --prefix is given as absolute path the library is installed into there
## use -q or --quiet to redirect build output to /dev/null instead of /dev/stdout
#
## NOTE: cURL depends on c-ares, so it must be built before cURL.
#

CURL_DIR=`pwd`

doclean=""
lprefix=""
stdout_target="/dev/stdout"
libPath="lib/.libs"
libcares="/tmp/installed-c-ares"
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -clean|--clean)
        doclean="yes"
        ;;
        -prefix|--prefix)
        lprefix="$2"
        libPath="${lprefix}/lib"
        libcares="$libPath"
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

GCC_can_build_x86_64="no"
GCC_can_build_arm64="no"
GCC_archs=`lipo -info "${GCCPATH}"`
if [[ "${GCC_archs}" = *"x86_64"* ]]; then GCC_can_build_x86_64="yes"; fi
if [[ "${GCC_archs}" = *"arm64"* ]]; then GCC_can_build_arm64="yes"; fi

if [ "${doclean}" != "yes" ]; then
    if [ -f "${libPath}/libcurl.a" ]; then
        alreadyBuilt=1

        if [ $GCC_can_build_x86_64 = "yes" ]; then
            lipo "${libPath}/libcurl.a" -verify_arch x86_64
            if [ $? -ne 0 ]; then alreadyBuilt=0; doclean="yes"; fi
        fi

        if [ $alreadyBuilt -eq 1 ] && [ $GCC_can_build_arm64 = "yes" ]; then
            lipo "${libPath}/libcurl.a" -verify_arch arm64
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
    rm -f "${libPath}/libcurl.a"
    if [ $? -ne 0 ]; then return 1; fi
fi

# c-ares configure creates a different ares_build.h file for each architecture
# for a sanity check on size of long and socklen_t. But these are  identical for
# x86_64 and arm64, so this is not currently an issue.
## cp -f ../"${caresDirName}"/ares_build_x86_64.h /tmp/installed-c-ares/include/ares_build.h

# Build for x86_64 architecture

export PATH=/usr/local/bin:$PATH
export CC="${GCCPATH}";export CXX="${GPPPATH}"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.13
export MAC_OS_X_VERSION_MAX_ALLOWED=101300
export MAC_OS_X_VERSION_MIN_REQUIRED=101300

export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64 -L${CURL_DIR}/../${opensslDirName} "
export CPPFLAGS="-isysroot ${SDKPATH} -arch x86_64 -mmacosx-version-min=10.13 -stdlib=libc++ -I${CURL_DIR}/../${opensslDirName}/include"
export CXXFLAGS="-isysroot ${SDKPATH} -arch x86_64 -mmacosx-version-min=10.13 -stdlib=libc++ -I${CURL_DIR}/../${opensslDirName}/include"
export CFLAGS="-isysroot ${SDKPATH} -mmacosx-version-min=10.13 -arch x86_64"

if [ "x${lprefix}" != "x" ]; then
    PKG_CONFIG_PATH="${lprefix}/lib/pkgconfig" ./configure --prefix=${lprefix} --enable-ares --disable-shared --with-openssl --without-apple-idn --without-libidn2 --without-libpsl --without-nghttp2 --without-ngtcp2 --without-nghttp3 --without-quiche --host=x86_64-apple-darwin
    if [ $? -ne 0 ]; then return 1; fi
else
    # Get the name of the current versions of c-ares from the
    # dependencyNames.sh file in the same directory as this script.
    myScriptPath="${BASH_SOURCE[0]}"
    if [ -z "${myScriptPath}" ]; then
        myScriptPath="$0"   # for zsh
    fi
    myScriptDir="${myScriptPath%/*}"
    source "${myScriptDir}/dependencyNames.sh"
    if [ $? -ne 0 ]; then return 1; fi

    # curl configure and make expect a path to _installed_ c-ares
    # so we temporarily installed c-ares at a path that does not contain spaces.
    # buildc-ares.sh installed c-ares to /tmp/installed-c-ares
    # and configured c-ares with prefix=/tmp/installed-c-ares
    if [ ! -f "${libcares}/libcares.a" ]; then
        cd ../"${caresDirName}" || return 1
        make install
        cd "${CURL_DIR}" || return 1
    fi

## Set flags again in case c-ares make install modified them
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64 -L${CURL_DIR}/../${opensslDirName} "
export CPPFLAGS="-isysroot ${SDKPATH} -arch x86_64 -mmacosx-version-min=10.13 -stdlib=libc++ -I${CURL_DIR}/../${opensslDirName}/include"
export CXXFLAGS="-isysroot ${SDKPATH} -arch x86_64 -mmacosx-version-min=10.13 -stdlib=libc++ -I${CURL_DIR}/../${opensslDirName}/include"
export CFLAGS="-isysroot ${SDKPATH} -mmacosx-version-min=10.13 -arch x86_64"
    ./configure --disable-shared --enable-ares="${libcares}" --with-openssl --without-apple-idn  --without-libidn2 --without-libpsl --without-nghttp2 --without-ngtcp2 --without-nghttp3 --without-quiche --host=x86_64-apple-darwin
    if [ $? -ne 0 ]; then return 1; fi
    echo ""
fi

if [ "${doclean}" = "yes" ]; then
    make clean
fi

make 1>$stdout_target
if [ $? -ne 0 ]; then return 1; fi

if [ "x${lprefix}" != "x" ]; then
    make install 1>$stdout_target
    if [ $? -ne 0 ]; then return 1; fi
fi

# Now see if we can build for arm64
# Note: Some versions of Xcode 12 don't support building for arm64
if [ $GCC_can_build_arm64 = "yes" ]; then

export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,arm64 -L${CURL_DIR}/../${opensslDirName} "
export CPPFLAGS="-isysroot ${SDKPATH} -target arm64-apple-macos -mmacosx-version-min=10.13 -stdlib=libc++ -I${CURL_DIR}/../${opensslDirName}/include"
export CXXFLAGS="-isysroot ${SDKPATH} -target arm64-apple-macos -mmacosx-version-min=10.13 -stdlib=libc++ -I${CURL_DIR}/../${opensslDirName}/include"
export CFLAGS="-isysroot ${SDKPATH} -mmacosx-version-min=10.13 -target arm64-apple-macos"

# c-ares configure creates a different ares_build.h file for each architecture
# for a sanity check on size of long and socklen_t. But these are  identical for
# x86_64 and arm64, so this is not currently an issue.
## cp -f ../"${caresDirName}"/ares_build_arm.h /tmp/installed-c-ares/include/ares_build.h
    if [ "x${lprefix}" != "x" ]; then
        PKG_CONFIG_PATH="${lprefix}/lib/pkgconfig" ./configure --prefix=${lprefix} --enable-ares --disable-shared --with-openssl --without-apple-idn --without-libidn2 --without-libpsl --without-nghttp2 --without-ngtcp2 --without-nghttp3 --without-quiche --host=arm-apple-darwin
    else
        ./configure --disable-shared --with-openssl --without-apple-idn --enable-ares="${libcares}" --without-libidn2 --without-libpsl --without-nghttp2 --without-ngtcp2 --without-nghttp3 --without-quiche --host=arm-apple-darwin
        echo ""
    fi

    if [ $? -ne 0 ]; then
        echo "              ******"
        echo "curl: x86_64 build succeeded but could not build for arm64."
        echo "              ******"
    else

        # save x86_64 header and lib for later use
        # curl configure creates a different curlbuild.h file for each architecture
        # for a sanity check on size of long and socklen_t. But these are  identical
        # for x86_64 and arm64, so this is not currently an issue.
    ##    cp -f include/curl/curlbuild.h include/curl/curlbuild_x86_64.h
        mv -f lib/.libs/libcurl.a lib/libcurl_x86_64.a

        make clean
        if [  $? -ne 0 ]; then return 1; fi

        make 1>$stdout_target
        if [ $? -ne 0 ]; then return 1; fi
        # curl configure creates a different curlbuild.h file for each architecture
        # for a sanity check on size of long and socklen_t. But these are  identical
        # for x86_64 and arm64, so this is not currently an issue.
    ##    mv -f include/curl/curlbuild.h include/curl/curlbuild_arm64.h
        mv -f lib/.libs/libcurl.a lib/libcurl_arm64.a

        lipo -create lib/libcurl_x86_64.a lib/libcurl_arm64.a -output lib/.libs/libcurl.a
        if [  $? -ne 0 ]; then
            rm -f lib/libcurl_x86_64.a lib/libcurl_arm64.a
        return 1
        fi
    fi

    rm -f lib/libcurl_x86_64.a lib/libcurl_arm64.a
fi

if [ "x${lprefix}" != "x" ]; then
    make install 1>$stdout_target
    if [ $? -ne 0 ]; then return 1; fi
else
    # Delete temporarily installed c-ares.
    rm -Rf ${libcares}
fi

export lprefix=""
export CC="";export CXX=""
export LDFLAGS=""
export CXXFLAGS=""
export CFLAGS=""
export SDKROOT=""

# curl configure creates a different curlbuild.h file for each architecture
# for a sanity check on size of long and socklen_t. But these are  identical
# for x86_64 and arm64, so this is not currently an issue and so we return now.
return 0

# Create a custom curlbuild.h file which directs BOINC builds
# to the correct curlbuild_xxx.h file for each architecture.
cat >> include/curl/curlbuild.h << ENDOFFILE
/***************************************************************************
*
* This file was created for BOINC by the buildcurl.sh script
*
* You should not need to modify it manually
*
 ***************************************************************************/

#ifndef __BOINC_CURLBUILD_H
#define __BOINC_CURLBUILD_H

#ifndef __APPLE__
#error - this file is for Macintosh only
#endif

#ifdef __x86_64__
#include "curl/curlbuild_x86_64.h"
#elif defined(__arm64__)
#include "curl/curlbuild_arm64.h"
#else
#error - unknown architecture
#endif

#endif /* __BOINC_CURLBUILD_H */
ENDOFFILE

return 0
