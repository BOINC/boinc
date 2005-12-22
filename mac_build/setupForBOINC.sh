#!/bin/sh

# Master script to build Universal Binary libraries needed by BOINC:
# curl-7.15.1, jpeg-6b and wxMac-2.6.2
#
# by Charlie Fenton 12/19/05
#
# Download these three packages and place them in a common parent 
# directory with the BOINC source tree.
#
## Change directory (cd) to boinc/mac_build/
## source ./setupForBoinc.sh [ -clean ]
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
source ${DIRSTACK[1]}/buildjpeg.sh
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
source ${DIRSTACK[1]}/buildcurl.sh
if [  $? -ne 0 ]; then exit 1; fi

echo ""
echo "----------------------------------"
echo "------- BUILD wxMac-2.6.2 --------"
echo "----------------------------------"
echo ""

popd
pushd ./

cd ../../wxMac-2.6.2/
if [  $? -ne 0 ]; then exit 1; fi
source ${DIRSTACK[1]}/buildWxMac.sh
if [  $? -ne 0 ]; then exit 1; fi

popd
exit 0
