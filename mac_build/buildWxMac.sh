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
# Script to build the wxMac-2.8.0 library for BOINC as a Universal Binary
#
# by Charlie Fenton    7/21/06
# Updated for wx-Mac 2.8.0 1/19/07

#
## Before running this script, you must first copy the special XCode 
## project
##    boinc/mac_build/wxMac-BOINC.xcodeproj 
## to 
##    wxMac-2.8.0/src/
#
#
## In Terminal, CD to the wxMac-2.8.0 directory.
##    cd [path]/wxMac-2.8.0/
## then run this script:
##    source [ path_to_this_script ] [ -clean ]
##
## the -clean argument will force a full rebuild.
#


if [ "$1" = "-clean" ]; then
  doclean="clean "
else
  doclean=""
fi

mv -n include/wx/mac/setup.h include/wx/mac/setup_obs.h
cp -np include/wx/mac/setup0.h include/wx/mac/setup.h

# Create wx include directory if necessary
if [ ! -d src/build/include/wx ]; then
    mkdir -p src/build/include/wx
fi

cp -n include/wx/mac/setup0.h src/build/include/wx/setup.h


if [ "$1" != "-clean" ] && [ -f src/build/Deployment/libwx_mac_static.a ]; then
    echo "Deployment libwx_mac_static.a already built"
else
    xcodebuild -project src/wxMac-BOINC.xcodeproj -target static  -configuration Deployment $doclean build GCC_VERSION_ppc=3.3 MACOSX_DEPLOYMENT_TARGET_ppc=10.3 SDKROOT_ppc=/Developer/SDKs/MacOSX10.3.9.sdk

if [  $? -ne 0 ]; then return 1; fi
fi

if [ "$1" != "-clean" ] && [ -f src/build/Development/libwx_mac_static.a ]; then
    echo "Development libwx_mac_static.a already built"
else
    xcodebuild -project src/wxMac-BOINC.xcodeproj -target static  -configuration Development $doclean build GCC_VERSION_ppc=3.3 MACOSX_DEPLOYMENT_TARGET_ppc=10.3 SDKROOT_ppc=/Developer/SDKs/MacOSX10.3.9.sdk

if [  $? -ne 0 ]; then return 1; fi
fi

return 0

