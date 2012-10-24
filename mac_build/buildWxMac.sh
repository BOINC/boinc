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
# Script to build the wxMac-2.9.4 library for BOINC
#
# by Charlie Fenton    7/21/06
# Updated for wx-Mac 2.8.10 and Unicode 4/17/09
# Updated for OS 10.7 and XCode 4.1 with OS 10.4 compatibility 9/26/11
# Updated for partial OS 10.8 and XCode 4.5 compatibility 7/27/12
## NOTE: To run with XCode 4.5, you must first obtain a copy of the 
##  MacOSX10.6.sdk and copy it into the folder: 
##  /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/
#
## This script requires OS 10.6 or later
##
## In Terminal, CD to the wxMac-2.9.4 directory.
##    cd [path]/wxMac-2.9.4/
## then run this script:
##    source [ path_to_this_script ] [ -clean ]
##
## the -clean argument will force a full rebuild.
#

Path=$PWD
echo "${Path}" | grep " " > /dev/null 2>&1
if [ "$?" -eq "0" ]; then
    echo "**********************************************************"
    echo "**********************************************************"
    echo "**********                                      **********"
    echo "********** ERROR: Path must not contain spaces! **********"
    echo "**********                                      **********"
    echo "**********************************************************"
    echo "**********************************************************"
    echo "**********************************************************"
    return 1
fi

if [ "$1" = "-clean" ]; then
  doclean="clean "
else
  doclean=""
fi

xcodebuild -version -sdk macosx10.6 > /dev/null
if [ ! "$?" = "0" ]; then
    echo "ERROR: System 10.6 SDK is missing.  For details, see build instructions at"
    echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or http://boinc.berkeley.edu/trac/wiki/MacBuild"
    return 1
fi

if [ "$1" != "-clean" ] && [ -f build/osx/build/Release/libwx_osx_carbon_static.a ]; then
    echo "Release libwx_osx_carbon_static.a already built"
else

##    export DEVELOPER_SDK_DIR="/Developer/SDKs"
    ## We must override some of the build settings in wxWindows.xcodeproj 
    xcodebuild -project build/osx/wxcarbon.xcodeproj -target static -configuration Release $doclean build -sdk macosx10.6 GCC_VERSION=com.apple.compilers.llvmgcc42 MACOSX_DEPLOYMENT_TARGET_i386=10.4 ARCHS="i386" OTHER_CPLUSPLUSFLAGS="-DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -DwxUSE_UNICODE=1 -fvisibility=hidden -fvisibility-inlines-hidden"

if [  $? -ne 0 ]; then return 1; fi
fi

if [ "$1" != "-clean" ] && [ -f build/osx/build/Debug/libwx_osx_carbon_static.a ]; then
    echo "Debug libwx_osx_carbon_static.a already built"
else
##    export DEVELOPER_SDK_DIR="/Developer/SDKs"
    ## We must override some of the build settings in wxWindows.xcodeproj 
    xcodebuild -project build/osx/wxcarbon.xcodeproj -target static -configuration Debug $doclean build -sdk macosx10.6 GCC_VERSION=com.apple.compilers.llvmgcc42 MACOSX_DEPLOYMENT_TARGET_i386=10.4 ARCHS="i386" OTHER_CPLUSPLUSFLAGS="-DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -DwxUSE_UNICODE=1 -fvisibility=hidden -fvisibility-inlines-hidden"

if [  $? -ne 0 ]; then return 1; fi
fi

return 0

