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
#
## This script requires OS 10.8 or later
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
## NOTE: cURL depends on OpenSLL and c-ares, so they must be built before cURL.
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

if [ "${doclean}" != "yes" ]; then
    if [ -f "${libPath}/libcurl.a" ]; then
        cwd=$(pwd)
        dirname=${cwd##*/}
        echo "${dirname} already built"
        return 0
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
    rm -f "${libPath}/libcurl.a"
    if [ $? -ne 0 ]; then return 1; fi
fi

export PATH=/usr/local/bin:$PATH
export CC="${GCCPATH}";export CXX="${GPPPATH}"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.7
export MAC_OS_X_VERSION_MAX_ALLOWED=1070
export MAC_OS_X_VERSION_MIN_REQUIRED=1070

if [ "x${lprefix}" != "x" ]; then
    export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64"
    export CPPFLAGS=""
    export CXXFLAGS="-isysroot ${SDKPATH} -arch x86_64 -stdlib=libc++"
    export CFLAGS="-isysroot ${SDKPATH} -arch x86_64"
    PKG_CONFIG_PATH="${lprefix}/lib/pkgconfig" ./configure --prefix=${lprefix} --enable-ares --enable-shared=NO --without-libidn --without-libidn2 --without-nghttp2 --host=x86_64
    if [ $? -ne 0 ]; then return 1; fi
else
    # Get the names of the current versions of c-ares and openssl from
    # the dependencyNames.sh file in the same directory as this script.
    myScriptPath="${BASH_SOURCE[0]}"
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

    export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64 -L${CURL_DIR}/../${opensslDirName} "
    export CPPFLAGS=""
    export CXXFLAGS="-isysroot ${SDKPATH} -arch x86_64 -stdlib=libc++ -I${CURL_DIR}/../${opensslDirName}/include"
    export CFLAGS="-isysroot ${SDKPATH} -arch x86_64 -I${CURL_DIR}/../${opensslDirName}/include"
    ./configure --enable-shared=NO --enable-ares="${libcares}" --without-libidn --without-libidn2 --without-nghttp2 --host=x86_64
    if [ $? -ne 0 ]; then return 1; fi
    echo ""
fi

# Patch curl_config.h to not use clock_gettime(), which is
# defined in OS 10.12 SDK but was not available before OS 10.12.
# If building with an older SDK or an older version of Xcode, these
# patches will fail because config has already set our desired values.
cat >> /tmp/curl_config_h_diff1 << ENDOFFILE
--- lib/curl_config.h    2018-02-22 04:21:52.000000000 -0800
+++ lib/curl_config1.h.in    2018-02-22 04:29:56.000000000 -0800
@@ -141,5 +141,5 @@

 /* Define to 1 if you have the __builtin_available function. */
-#define HAVE_BUILTIN_AVAILABLE 1
+/* #undef HAVE_BUILTIN_AVAILABLE */

 /* Define to 1 if you have the clock_gettime function and monotonic timer. */
ENDOFFILE

patch -fi /tmp/curl_config_h_diff1 lib/curl_config.h
rm -f /tmp/curl_config_h_diff1
rm -f lib/curl_config.h.rej

cat >> /tmp/curl_config_h_diff2 << ENDOFFILE
--- lib/curl_config.h    2018-02-22 04:21:52.000000000 -0800
+++ lib/curl_config2.h.in    2018-02-22 04:30:21.000000000 -0800
@@ -144,5 +144,5 @@

 /* Define to 1 if you have the clock_gettime function and monotonic timer. */
-#define HAVE_CLOCK_GETTIME_MONOTONIC 1
+/* #undef HAVE_CLOCK_GETTIME_MONOTONIC */

 /* Define to 1 if you have the closesocket function. */
ENDOFFILE

patch -fi /tmp/curl_config_h_diff2 lib/curl_config.h
rm -f /tmp/curl_config_h_diff2
rm -f lib/curl_config.h.rej

if [ "${doclean}" = "yes" ]; then
    make clean
fi

make 1>$stdout_target
if [ $? -ne 0 ]; then return 1; fi

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

return 0
