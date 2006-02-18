#!/bin/sh

# Berkeley Open Infrastructure for Network Computing
# http://boinc.berkeley.edu
# Copyright (C) 2005 University of California
#
# This is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation;
# either version 2.1 of the License, or (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# To view the GNU Lesser General Public License visit
# http://www.gnu.org/copyleft/lesser.html
# or write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#
# Master script to build Universal Binary libraries needed by BOINC:
# curl-7.15.1, jpeg-6b and wxMac-2.6.2
#
# by Charlie Fenton 2/17/06
#
# Download these three packages and place them in a common parent 
# directory with the BOINC source tree.
#
# Before running this script, you must first prepare the wxMac-2.6.2 
# package.  Read and follow the directions for this in the script 
# buildWxMac.sh; it is in the same folder as this script.
#
## In Terminal, CD to the wxMac-2.6.2 directory.
##     cd [path]/boinc/mac_build/
## then run this script:
##     source setupForBoinc.sh [ -clean ]
#
# the -clean argument will force a full rebuild of everything.
#
# This script will work even if you have renamed the boinc/ directory
#


pushd ./

echo ""
echo "----------------------------------"
echo "--------- BUILD JPEG-6B ----------"
echo "----------------------------------"
echo ""

cd ../../jpeg-6b/
if [  $? -ne 0 ]; then exit 1; fi
source "${DIRSTACK[1]}/buildjpeg.sh"
if [  $? -ne 0 ]; then exit 1; fi

echo ""
echo "----------------------------------"
echo "------- BUILD CURL-7.15.1 --------"
echo "----------------------------------"
echo ""

popd
pushd ./

cd ../../curl-7.15.1/
if [  $? -ne 0 ]; then exit 1; fi
source "${DIRSTACK[1]}/buildcurl.sh"
if [  $? -ne 0 ]; then exit 1; fi

echo ""
echo "----------------------------------"
echo "------- BUILD wxMac-2.6.2 --------"
echo "----------------------------------"
echo ""

popd
pushd ./

cp -fpR wxMac-BOINC.xcodeproj ../../wxMac-2.6.2/src/

cd ../../wxMac-2.6.2/
if [  $? -ne 0 ]; then exit 1; fi
source "${DIRSTACK[1]}/buildWxMac.sh"
if [  $? -ne 0 ]; then exit 1; fi

popd
exit 0
