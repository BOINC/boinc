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
# Script to build the wxMac-2.8.3 library for BOINC as a Universal Binary
#
# by Charlie Fenton    7/21/06
# Updated for wx-Mac 2.8.3 4/12/07

#
## In Terminal, CD to the wxMac-2.8.3 directory.
##    cd [path]/wxMac-2.8.3/
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

if [ "$1" != "-clean" ] && [ -f src/build/Deployment/libwx_mac_static.a ]; then
    echo "Deployment libwx_mac_static.a already built"
else
    xcodebuild -project src/wxWindows.xcodeproj -target static -configuration Deployment $doclean build GCC_VERSION_ppc=3.3 MACOSX_DEPLOYMENT_TARGET_ppc=10.3 SDKROOT_ppc=/Developer/SDKs/MacOSX10.3.9.sdk

if [  $? -ne 0 ]; then return 1; fi
fi

if [ "$1" != "-clean" ] && [ -f src/build/Development/libwx_mac_static.a ]; then
    echo "Development libwx_mac_static.a already built"
else
    xcodebuild -project src/wxWindows.xcodeproj -target static -configuration Development $doclean build GCC_VERSION_ppc=3.3 MACOSX_DEPLOYMENT_TARGET_ppc=10.3 SDKROOT_ppc=/Developer/SDKs/MacOSX10.3.9.sdk
## The above line does Development build for only the native architecture.  
## Use line below instead for Universal Binary Development build
##    xcodebuild -project src/wxWindows.xcodeproj -target static -configuration Development $doclean build ARCHS="i386 ppc" GCC_VERSION_ppc=3.3 MACOSX_DEPLOYMENT_TARGET_ppc=10.3 SDKROOT_ppc=/Developer/SDKs/MacOSX10.3.9.sdk

if [  $? -ne 0 ]; then return 1; fi
fi

return 0

