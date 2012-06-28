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
# Updated for OS 10.7 and XCode 4.1 with OS 10.4 compatibility 9/26/11
# Updated for partial OS 10.8 and XCode 4.5 compatibility 6/28/12
## NOTE: To run with XCode 4.5, you must first obtain a copy of the 
##  MacOSX10.6.sdk and copy it into the folder: 
##  /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/
#
## This script requires OS 10.6 or later
##
## In Terminal, CD to the wxMac-2.8.10 directory.
##    cd [path]/wxMac-2.8.10/
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

xcodebuild -version -sdk macosx10.6
if [ ! "$?" = "0" ]; then
    echo "ERROR: System 10.6 SDK is missing.  For details, see build instructions at"
    echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or http://boinc.berkeley.edu/trac/wiki/MacBuild"
    return 1
fi

## By default, wxMac 2.8.10 links with libiconv.dyld, but building with OS 10.6 SDK 
## makes it require libiconv.2.4.0.dylib, while OS 10.4 supplies only libiconv.2.2.0.dylib.
## This causes the Manager to fail under OS 10.4 with a dyld error.  
## But libiconv is not really needed for BOINC Manager, so we patch wxMac's config file to 
## avoid using libiconv.
if [ ! -f "include/wx/mac/carbon/config_xcode.h.orig" ]; then
    ## Make sure sed uses UTF-8 text encoding
    unset LC_CTYPE
    unset LC_MESSAGES
    unset __CF_USER_TEXT_ENCODING
    export LANG=en_US.UTF-8

    echo "patching file include/wx/mac/carbon/config_xcode.h"
    sed -i ".orig" s%"#define HAVE_ICONV 1"%"//#undef HAVE_ICONV"%g "include/wx/mac/carbon/config_xcode.h"
else
    echo "config_xcode.h already patched"
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
    ## We must override some of the build settings in wxWindows.xcodeproj 
    xcodebuild -project src/wxWindows.xcodeproj -target static -configuration Deployment $doclean build -sdk macosx10.6 GCC_VERSION_i386=com.apple.compilers.llvmgcc42 MACOSX_DEPLOYMENT_TARGET_i386=10.4 ARCHS="i386" OTHER_CPLUSPLUSFLAGS="-DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -DwxUSE_UNICODE=1 -fvisibility=hidden -fvisibility-inlines-hidden"

if [  $? -ne 0 ]; then return 1; fi
fi

if [ "$1" != "-clean" ] && [ -f src/build/Development/libwx_mac_static.a ]; then
    echo "Development libwx_mac_static.a already built"
else
    export DEVELOPER_SDK_DIR="/Developer/SDKs"
    ## We must override some of the build settings in wxWindows.xcodeproj 
    xcodebuild -project src/wxWindows.xcodeproj -target static -configuration Development $doclean build -sdk macosx10.6 GCC_VERSION_i386=com.apple.compilers.llvmgcc42 MACOSX_DEPLOYMENT_TARGET_i386=10.4 ARCHS="i386" OTHER_CPLUSPLUSFLAGS="-DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -DwxUSE_UNICODE=1 -fvisibility=hidden -fvisibility-inlines-hidden"

if [  $? -ne 0 ]; then return 1; fi
fi

return 0

