#!/bin/zsh

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
# Master script to build Universal Binary libraries needed by BOINC:
# curl-7.47.1 with c-ares-1.10.0, openssl-1.1.0, wxWidgets-3.0.0,
# sqlite-3.11.0, FreeType-2.4.10, FTGL-2.1.3 and libzip-1.11.4
#
# by Charlie Fenton 7/21/06
# Updated 10/18/11 for OS 10.7 lion and XCode 4.2
# Updated 7/6/11 for wxMac-2.8.10 and Unicode
# Updated 6/25/12 for curl-7.26.0 and c-ares-1.9.1
# Updated 6/26/12 for openssl-1.0.1c
# Updated 8/3/12 for FreeType-2.4.10 and FTGL-2.1.3~rc5
# Updated 12/11/12 for sqlite3.7.14.1 from sqlite-autoconf-3071401
# Updated 11/30/13 for openssl-1.0.1e
# Updated 2/7/14 for wxWidgets-3.0.0
# Updated 2/11/14 for c-ares 1.10.0, curl 7.35.0, openssl 1.0.1f, sqlite 3.8.3
# Updated 9/2/14 for openssl 1.0.1h
# Updated 4/8/15 for curl 7.39.0, openssl 1.0.1j
# Updated 11/30/15 to allow putting third party packages in ../mac3rdParty/
# Updated 11/30/15 to return error code indicating which builds failed
# Updated 1/6/16 for curl 7.46.0, openssl 1.0.2e, sqlite 3.9.2, FreeType-2.6.2
# Updated 3/2/16 for curl 7.47.1, openssl 1.0.2g, sqlite 3.11.0
# Updated 9/10/16 for c-ares 1.11.0, curl 7.50.2, openssl 1.1.0
# Updated 6/25/23 to download inflate libraries if needed
# Updated 9/25/25 for libzip-1.11.4
#
# Download these seven packages and place them in a common parent directory
# with the BOINC source tree.
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
  cleanit="-clean"
else
  cleanit=""
fi

caresOK="NO"
curlOK="NO"
opensslOK="NO"
wxWidgetsOK="NO"
freetypeOK="NO"
ftglOK="NO"
zipOK="NO"
finalResult=0

SCRIPT_DIR=`pwd`

# this will pull in the variables used below
source "${SCRIPT_DIR}/dependencyNames.sh"

# Dowload and expand libraries if needed
for rootName in "openssl" "cares" "curl" "wxWidgets" "freetype" "ftgl" "zip"
do
    dirVar="${rootName}DirName"
    fileVar="${rootName}FileName"
    urlVar="${rootName}URL"
    # Test whether shell is zsh or bash
    if [ -z "${BASH_VERSINFO}" ]; then
        # zsh commands
        dirName="${(P)dirVar}"
        fileName="${(P)fileVar}"
        URLString="${(P)urlVar}"
    else
        # bash commands
        dirName="${!dirVar}"
        fileName="${!fileVar}"
        URLString="${!urlVar}"
    fi

    cd ../../
    echo
    if [ -d "${dirName}" ]; then
    echo `pwd`"/${dirName}"" already present"
    else
        if [ -e "${fileName}" ]; then
            echo `pwd`"/""${fileName}"" already present"
        else
            echo "Downloading ""${fileName}"" to "`pwd`"/"
            curl -L -O "${URLString}"
            echo
        fi
            echo "Expanding ""${fileName}"" to ""${dirName}" in `pwd`"/"
        tar -xf "${fileName}"
        if [ $? -ne 0 ]; then
            echo;echo "**************************************"
            echo "ERROR: can't expand " "${dirName}"
            echo "**************************************";echo
            cd "${SCRIPT_DIR}"
            return 1
        fi
    fi
    cd "${SCRIPT_DIR}"
done

echo ""
echo "----------------------------------"
echo "--------- BUILD OPENSSL ----------"
echo "----------------------------------"
echo ""

cd "../../${opensslDirName}"
if [  $? -eq 0 ]; then
    source "${SCRIPT_DIR}/buildopenssl.sh" ${cleanit}
    if [  $? -eq 0 ]; then
        opensslOK="YES"
    fi
fi

cd "${SCRIPT_DIR}"

echo ""
echo "----------------------------------"
echo "---------- BUILD C-ARES- ---------"
echo "----------------------------------"
echo ""

cd "../../${caresDirName}"
if [  $? -eq 0 ]; then
    source "${SCRIPT_DIR}/buildc-ares.sh" ${cleanit}
    if [  $? -eq 0 ]; then
        caresOK="YES"
    fi
fi

cd "${SCRIPT_DIR}"

echo ""
echo "----------------------------------"
echo "----------- BUILD CURL -----------"
echo "----------------------------------"
echo ""

cd "../../${curlDirName}"
if [  $? -eq 0 ]; then
    source "${SCRIPT_DIR}/buildcurl.sh" ${cleanit}
    if [  $? -eq 0 ]; then
        curlOK="YES"
    fi
fi

cd "${SCRIPT_DIR}"

echo ""
echo "----------------------------------"
echo "-------- BUILD wxWidgets ---------"
echo "----------------------------------"
echo ""

cd "../../${wxWidgetsDirName}"
if [  $? -eq 0 ]; then
    source "${SCRIPT_DIR}/buildWxMac.sh" ${cleanit}
    if [  $? -eq 0 ]; then
        wxWidgetsOK="YES"
    fi
fi

cd "${SCRIPT_DIR}"

echo ""
echo "----------------------------------"
echo "--------- BUILD FreeType ---------"
echo "----------------------------------"
echo ""

cd "../../${freetypeDirName}"
if [  $? -eq 0 ]; then
    source "${SCRIPT_DIR}/buildfreetype.sh" ${cleanit}
    if [  $? -eq 0 ]; then
        freetypeOK="YES"
    fi
fi

cd "${SCRIPT_DIR}"

echo ""
echo "----------------------------------"
echo "----------- BUILD FTGL -----------"
echo "----------------------------------"
echo ""

cd "../../${ftglDirName}"
if [  $? -eq 0 ]; then
    source "${SCRIPT_DIR}/buildFTGL.sh" ${cleanit}
    if [  $? -eq 0 ]; then
        ftglOK="YES"
    fi
fi

cd "${SCRIPT_DIR}"

if [ -x /usr/local/bin/ccmake ]; then
    echo ""
    echo "----------------------------------"
    echo "----------- BUILD ZIP -----------"
    echo "----------------------------------"
    echo ""

    cd "../../${zipDirName}"
    if [  $? -eq 0 ]; then
        source "${SCRIPT_DIR}/buildlibzip.sh" ${cleanit}
        if [  $? -eq 0 ]; then
            zipOK="YES"
        fi
    fi

    cd "${SCRIPT_DIR}"
else
    echo ""
    echo "----------------------------------"
    echo "--- TO BUILD ZIP PLEASE INSTALL --"
    echo "----  CMAKE COMMAND-LINE TOOLS ----"
    echo "----------------------------------"
    echo ""
fi

if [ "${caresOK}" = "NO" ]; then
    echo ""
    echo "-----------------------------------"
    echo "------------ WARNING --------------"
    echo "------------         --------------"
    echo "-- COULD NOT BUILD ${caresDirName} --"
    echo "-----------------------------------"
    echo ""

    finalResult=$[ finalResult | 1 ]
fi

if [ "${curlOK}" = "NO" ]; then
    echo ""
    echo "-----------------------------------"
    echo "------------ WARNING --------------"
    echo "------------         --------------"
    echo "--- COULD NOT BUILD ${curlDirName} ---"
    echo "-----------------------------------"
    echo ""

    finalResult=$[ finalResult | 2 ]
fi

if [ "${opensslOK}" = "NO" ]; then
    echo ""
    echo "----------------------------------"
    echo "------------ WARNING -------------"
    echo "------------         -------------"
    echo "- COULD NOT BUILD ${opensslDirName} -"
    echo "----------------------------------"
    echo ""

    finalResult=$[ finalResult | 4 ]
fi

if [ "${wxWidgetsOK}" = "NO" ]; then
    echo ""
    echo "-----------------------------------"
    echo "------------ WARNING --------------"
    echo "------------         --------------"
    echo "- COULD NOT BUILD ${wxWidgetsDirName} -"
    echo "-----------------------------------"
    echo ""

    finalResult=$[ finalResult | 8 ]
fi

if [ "${freetypeOK}" = "NO" ]; then
    echo ""
    echo "-----------------------------------"
    echo "------------ WARNING --------------"
    echo "------------         --------------"
    echo "- COULD NOT BUILD ${freetypeDirName} -"
    echo "-----------------------------------"
    echo ""

    finalResult=$[ finalResult | 32 ]
fi

if [ "${ftglOK}" = "NO" ]; then
    echo ""
    echo "-----------------------------------"
    echo "------------ WARNING --------------"
    echo "------------         --------------"
    echo "- COULD NOT BUILD ${ftglDirName} --"
    echo "-----------------------------------"
    echo ""

    finalResult=$[ finalResult | 64 ]
fi

if [ "${zipOK}" = "NO" ]; then
    echo ""
    echo "-----------------------------------"
    echo "------------ WARNING --------------"
    echo "------------         --------------"
    echo "- COULD NOT BUILD ${zipDirName} --"
    echo "-----------------------------------"
    echo ""

    finalResult=$[ finalResult | 128 ]
fi

echo ""

return $finalResult
