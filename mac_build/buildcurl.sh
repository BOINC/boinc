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
# Script to build Macintosh 32-bit Intel library of curl-7.50.2 for
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
#
## This script requires OS 10.6 or later
#
## If you drag-install Xcode 4.3 or later, you must have opened Xcode
## and clicked the Install button on the dialog which appears to
## complete the Xcode installation before running this script.
#
## In Terminal, CD to the curl-7.50.2 directory.
##     cd [path]/curl-7.50.2/
## then run this script:
##     source [path]/buildcurl.sh [ -clean ] [--prefix PATH]
##
## the -clean argument will force a full rebuild.
## if --prefix is given as absolute path the library is installed into there
## use -q or --quiet to redirect build output to /dev/null instead of /dev/stdout
##

CURL_DIR=`pwd`

# Patch curl-7.50.2/include/curl/curlrules.h so it doesn't
# cause our 32-bit build of BOINC Manager to fail.
if [ ! -f include/curl/curlrules.h.orig ]; then
    cat >> /tmp/curlrules_h_diff << ENDOFFILE
--- /Volumes/Cheer/BOINC_GIT/curl-7.50.2/include/curl/curlrules_orig.h	2016-08-08 05:03:14.000000000 -0700
+++ /Volumes/Cheer/BOINC_GIT/curl-7.50.2/include/curl/curlrules.h	2017-03-13 17:17:43.000000000 -0700
@@ -74,6 +74,7 @@
 /*
  * Verify that some macros are actually defined.
  */
+#if 0

 #ifndef CURL_SIZEOF_LONG
 #  error "CURL_SIZEOF_LONG definition is missing!"
@@ -182,6 +183,8 @@
   __curl_rule_05__
     [CurlchkszGE(curl_socklen_t, int)];

+#endif
+
 /* ================================================================ */
 /*          EXTERNALLY AND INTERNALLY VISIBLE DEFINITIONS           */
 /* ================================================================ */
ENDOFFILE
    patch -bfi /tmp/curlrules_h_diff include/curl/curlrules.h
    rm -f /tmp/curlrules_h_diff
else
    echo "include/curl/curlrules.h already patched"
fi

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
export MACOSX_DEPLOYMENT_TARGET=10.6
export MAC_OS_X_VERSION_MAX_ALLOWED=1060
export MAC_OS_X_VERSION_MIN_REQUIRED=1060

if [ "x${lprefix}" != "x" ]; then
    export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64"
    export CPPFLAGS="-isysroot ${SDKPATH} -arch x86_64"
    export CFLAGS="-isysroot ${SDKPATH} -arch x86_64"
    PKG_CONFIG_PATH="${lprefix}/lib/pkgconfig" ./configure --prefix=${lprefix} --enable-ares --enable-shared=NO --host=x86_64
    if [ $? -ne 0 ]; then return 1; fi
else
    # curl configure and make expect a path to _installed_ c-ares-1.11.0
    # so we temporarily installed c-ares at a path that does not contain spaces.
    # buildc-ares.sh installed c-ares to /tmp/installed-c-ares
    # and configured c-ares with prefix=/tmp/installed-c-ares
    if [ ! -f "${libcares}/libcares.a" ]; then
        cd ../c-ares-1.11.0 || return 1
        make install
        cd "${CURL_DIR}" || return 1
    fi

    export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64 -L${CURL_DIR}/../openssl-1.1.0 "
    export CPPFLAGS="-isysroot ${SDKPATH} -arch x86_64 -I${CURL_DIR}/../openssl-1.1.0/include"
    export CFLAGS="-isysroot ${SDKPATH} -arch x86_64 -I${CURL_DIR}/../openssl-1.1.0/include"
    ./configure --enable-shared=NO --enable-ares="${libcares}" --host=x86_64
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
else
    # Delete temporarily installed c-ares.
    rm -Rf ${libcares}
fi

export lprefix=""
export CC="";export CXX=""
export LDFLAGS=""
export CPPFLAGS=""
export CFLAGS=""
export SDKROOT=""

return 0
