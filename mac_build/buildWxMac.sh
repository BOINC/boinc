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
# Script to build the wxMac-2.8.10 library for BOINC as a Universal Binary
#
# by Charlie Fenton    7/21/06
# Updated for wx-Mac 2.8.10 and Unicode 4/17/09

#
## In Terminal, CD to the wxMac-2.8.10 directory.
##    cd [path]/wxMac-2.8.10/
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

if [ ! -d /Developer/SDKs/MacOSX10.4u.sdk/ ]; then
    echo "ERROR: System 10.4u SDK is missing.  For details, see build instructions at"
    echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or http://boinc.berkeley.edu/trac/wiki/MacBuild"
    return 1
fi

if [ ! -d /Developer/SDKs/MacOSX10.5.sdk/ ]; then
    echo "ERROR: System 10.5 SDK is missing.  For details, see build instructions at"
    echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or http://boinc.berkeley.edu/trac/wiki/MacBuild"
    return 1
fi

## Fix a bug in wxMutex.  This patch should not be needed for wxMac-2.8.11 and later
if [ ! -f src/mac/carbon/thread.cpp.orig ]; then

cat >> /tmp/wxmutex_diff << ENDOFFILE
--- src/mac/carbon/thread-old.cpp	2009-03-06 04:23:14.000000000 -0800
+++ src/mac/carbon/thread.cpp	2009-05-05 04:34:41.000000000 -0700
@@ -138,8 +138,8 @@
 
 #if TARGET_API_MAC_OSX
 #define wxUSE_MAC_SEMAPHORE_MUTEX 0
-#define wxUSE_MAC_CRITICAL_REGION_MUTEX 1
-#define wxUSE_MAC_PTHREADS_MUTEX 0
+#define wxUSE_MAC_CRITICAL_REGION_MUTEX 0
+#define wxUSE_MAC_PTHREADS_MUTEX 1
 #else
 #define wxUSE_MAC_SEMAPHORE_MUTEX 0
 #define wxUSE_MAC_CRITICAL_REGION_MUTEX 1
ENDOFFILE

patch -bfi /tmp/wxmutex_diff src/mac/carbon/thread.cpp

rm -f /tmp/wxmutex_diff
else
    echo "thread.cpp already patched"
fi

if [ "$1" != "-clean" ] && [ -f src/build/Deployment/libwx_mac_static.a ]; then
    echo "Deployment libwx_mac_static.a already built"
else
     export DEVELOPER_SDK_DIR="/Developer/SDKs"
     xcodebuild -project src/wxWindows.xcodeproj -target static -configuration Deployment $doclean build GCC_VERSION=4.0 GCC_VERSION_ppc=4.0 MACOSX_DEPLOYMENT_TARGET=10.4 MACOSX_DEPLOYMENT_TARGET_ppc=10.4 SDKROOT=/Developer/SDKs/MacOSX10.4u.sdk SDKROOT_ppc=/Developer/SDKs/MacOSX10.4u.sdk ARCHS="i386 ppc" OTHER_CPLUSPLUSFLAGS="-DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -DwxUSE_UNICODE=1 -fvisibility=hidden -fvisibility-inlines-hidden"

if [  $? -ne 0 ]; then return 1; fi
fi

if [ "$1" != "-clean" ] && [ -f src/build/Development/libwx_mac_static.a ]; then
    echo "Development libwx_mac_static.a already built"
else
     export DEVELOPER_SDK_DIR="/Developer/SDKs"
     xcodebuild -project src/wxWindows.xcodeproj -target static -configuration Development $doclean build GCC_VERSION=4.0 GCC_VERSION_ppc=4.0 MACOSX_DEPLOYMENT_TARGET=10.4 MACOSX_DEPLOYMENT_TARGET_ppc=10.4 SDKROOT=/Developer/SDKs/MacOSX10.4u.sdk SDKROOT_ppc=/Developer/SDKs/MacOSX10.4u.sdk OTHER_CPLUSPLUSFLAGS="-DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -DwxUSE_UNICODE=1 -fvisibility=hidden -fvisibility-inlines-hidden"
## The above line does Development build for only the native architecture.  
## Use line below instead for Universal Binary Development build
##    xcodebuild -project src/wxWindows.xcodeproj -target static -configuration Development $doclean build MACOSX_DEPLOYMENT_TARGET=10.4 SDKROOT=/Developer/SDKs/MacOSX10.4u.sdk ARCHS="i386 ppc" OTHER_CPLUSPLUSFLAGS="-DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -fvisibility=hidden -fvisibility-inlines-hidden"

if [  $? -ne 0 ]; then return 1; fi
fi

return 0

