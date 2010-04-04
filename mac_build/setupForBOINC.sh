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
# curl-7.19.7 with c-ares-1.6.0, jpeg-6b and wxMac-2.8.10
#
# Note: reverted to c-ares 1.6.0 from 1.7.0 because the newer c-ares has 
# problems resolving host names on OS 10.6 with default settings when used 
# with AT&T U-Verse 2Wire gateway routers and Airport.
#
# by Charlie Fenton 7/21/06
# Updated 4/3/10 for curl-7.19.7, c-ares-1.6.0 and wxMac-2.8.10 and Unicode
#
# Download these three packages and place them in a common parent 
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

SCRIPT_DIR=`pwd`

echo ""
echo "----------------------------------"
echo "------- BUILD C-ARES-1.6.0 -------"
echo "----------------------------------"
echo ""

cd ../../c-ares-1.6.0/
if [  $? -ne 0 ]; then return 1; fi
source "${SCRIPT_DIR}/buildc-ares.sh" ${doclean}
if [  $? -ne 0 ]; then return 1; fi

echo ""
echo "----------------------------------"
echo "------- BUILD CURL-7.19.7 --------"
echo "----------------------------------"
echo ""

cd "${SCRIPT_DIR}"

cd ../../curl-7.19.7/
if [  $? -ne 0 ]; then return 1; fi
source "${SCRIPT_DIR}/buildcurl.sh" ${doclean}
if [  $? -ne 0 ]; then return 1; fi

echo ""
echo "----------------------------------"
echo "--------- BUILD JPEG-6B ----------"
echo "----------------------------------"
echo ""

cd "${SCRIPT_DIR}"

cd ../../jpeg-6b/
if [  $? -ne 0 ]; then return 1; fi
source "${SCRIPT_DIR}/buildjpeg.sh" ${doclean}
if [  $? -ne 0 ]; then return 1; fi

echo ""
echo "----------------------------------"
echo "------- BUILD wxMac-2.8.10 --------"
echo "----------------------------------"
echo ""

cd "${SCRIPT_DIR}"

cd ../../wxMac-2.8.10/
if [  $? -ne 0 ]; then return 1; fi
source "${SCRIPT_DIR}/buildWxMac.sh" ${doclean}
if [  $? -ne 0 ]; then return 1; fi

cd "${SCRIPT_DIR}"
return 0
