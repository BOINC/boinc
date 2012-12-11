#!/bin/sh

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2008 University of California
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
# Master script to build Universal Binary libraries needed by BOINC:
# curl-7.26.0 with c-ares-1.9.1, openssl-1.0.1c, wxMac-2.8.10,
# sqlite3.7.14.1, FreeType-2.4.10 and FTGL-2.1.3
#
# by Charlie Fenton 7/21/06
# Updated 10/18/11 for OS 10.7 lion and XCode 4.2
# Updated 7/6/11 for wxMac-2.8.10 and Unicode
# Updated 6/25/12 for curl-7.26.0 and c-ares-1.9.1
# Updated 6/26/12 for openssl-1.0.1c
# Updated 8/3/12 for FreeType-2.4.10 and FTGL-2.1.3~rc5
# Updated 12/11/12 for sqlite3.7.14.1 from sqlite-autoconf-3071401
#
# Download these seven packages and place them in a common parent 
# directory with the BOINC source tree.
#
## In Terminal, cd to the mac_build directory of the boinc tree; for 
## example:
##     cd [path]/boinc/mac_build/
## then run this script:
##     source setupForBoinc.sh [ -clean ]
#
# the -clean argument will force a full rebuild of everything.
#
# This script will work even if you have renamed the boinc/ directory
#

if [ "$1" = "-clean" ]; then
  doclean="-clean"
else
  doclean=""
fi

caresOK="NO"
curlOK="NO"
opensslOK="NO"
wxWidgetsOK="NO"
sqlite3OK="NO"
freetypeOK="NO"
ftglOK="NO"

SCRIPT_DIR=`pwd`

echo ""
echo "----------------------------------"
echo "------- BUILD C-ARES-1.9.1 -------"
echo "----------------------------------"
echo ""

cd ../../c-ares-1.9.1/
if [  $? -eq 0 ]; then
    source "${SCRIPT_DIR}/buildc-ares.sh" ${doclean}
    if [  $? -eq 0 ]; then
        caresOK="YES"
    fi
fi

echo ""
echo "----------------------------------"
echo "------- BUILD CURL-7.26.0 --------"
echo "----------------------------------"
echo ""

cd "${SCRIPT_DIR}"

cd ../../curl-7.26.0/
if [  $? -eq 0 ]; then
    source "${SCRIPT_DIR}/buildcurl.sh" ${doclean}
    if [  $? -eq 0 ]; then
        curlOK="YES"
    fi
fi

echo ""
echo "----------------------------------"
echo "----- BUILD OPENSSL-1.0.1c -------"
echo "----------------------------------"
echo ""

cd "${SCRIPT_DIR}"

cd ../../openssl-1.0.1c/
if [  $? -eq 0 ]; then
    source "${SCRIPT_DIR}/buildopenssl.sh" ${doclean}
    if [  $? -eq 0 ]; then
        opensslOK="YES"
    fi
fi

echo ""
echo "----------------------------------"
echo "------- BUILD wxMac-2.8.10 -------"
echo "----------------------------------"
echo ""

cd "${SCRIPT_DIR}"

cd ../../wxMac-2.8.10/
if [  $? -eq 0 ]; then
    source "${SCRIPT_DIR}/buildWxMac.sh" ${doclean}
    if [  $? -eq 0 ]; then
        wxWidgetsOK="YES"
    fi
fi

echo ""
echo "----------------------------------"
echo "------ BUILD sqlite3.7.14.1 ------"
echo "----------------------------------"
echo ""

cd "${SCRIPT_DIR}"

cd ../../sqlite-autoconf-3071401/
if [  $? -eq 0 ]; then
    source "${SCRIPT_DIR}/buildsqlite3.sh" ${doclean}
    if [  $? -eq 0 ]; then
        sqlite3OK="YES"
    fi
fi

echo ""
echo "----------------------------------"
echo "----- BUILD FreeType-2.4.10 ------"
echo "----------------------------------"
echo ""

cd "${SCRIPT_DIR}"

cd ../../freetype-2.4.10/
if [  $? -eq 0 ]; then
    source "${SCRIPT_DIR}/buildfreetype.sh" ${doclean}
    if [  $? -eq 0 ]; then
        freetypeOK="YES"
    fi
fi

echo ""
echo "----------------------------------"
echo "------ BUILD FTGL-2.1.3~rc5 ------"
echo "----------------------------------"
echo ""

cd "${SCRIPT_DIR}"

cd ../../ftgl-2.1.3~rc5/
if [  $? -eq 0 ]; then
    source "${SCRIPT_DIR}/buildFTGL.sh" ${doclean}
    if [  $? -eq 0 ]; then
        ftglOK="YES"
    fi
fi

if [ "${caresOK}" = "NO" ]; then
    echo ""
    echo "----------------------------------"
    echo "------------ WARNING -------------"
    echo "------------         -------------"
    echo "-- COULD NOT BUILD C-ARES-1.9.1 --"
    echo "----------------------------------"
    echo ""
fi

if [ "${curlOK}" = "NO" ]; then
    echo ""
    echo "-----------------------------------"
    echo "------------ WARNING --------------"
    echo "------------         --------------"
    echo "--- COULD NOT BUILD CURL-7.26.0 ---"
    echo "-----------------------------------"
    echo ""
fi

if [ "${opensslOK}" = "NO" ]; then
    echo ""
    echo "----------------------------------"
    echo "------------ WARNING -------------"
    echo "------------         -------------"
    echo "- COULD NOT BUILD OPENSSL-1.0.1c -"
    echo "----------------------------------"
    echo ""
fi

if [ "${wxWidgetsOK}" = "NO" ]; then
    echo ""
    echo "----------------------------------"
    echo "------------ WARNING -------------"
    echo "------------         -------------"
    echo "-- COULD NOT BUILD wxMac-2.8.10 --"
    echo "----------------------------------"
    echo ""
fi

if [ "${sqlite3OK}" = "NO" ]; then
    echo ""
    echo "----------------------------------"
    echo "------------ WARNING -------------"
    echo "------------         -------------"
    echo "- COULD NOT BUILD sqlite3.7.14.1 -"
    echo "----------------------------------"
    echo ""
fi

if [ "${freetypeOK}" = "NO" ]; then
    echo ""
    echo "-----------------------------------"
    echo "------------ WARNING --------------"
    echo "------------         -------------"
    echo "- COULD NOT BUILD FreeType-2.4.10 -"
    echo "-----------------------------------"
    echo ""
fi

if [ "${ftglOK}" = "NO" ]; then
    echo ""
    echo "-----------------------------------"
    echo "------------ WARNING --------------"
    echo "------------         -------------"
    echo "- COULD NOT BUILD FTGL-2.1.3~rc50 -"
    echo "-----------------------------------"
    echo ""
fi

echo ""
cd "${SCRIPT_DIR}"

return 0
