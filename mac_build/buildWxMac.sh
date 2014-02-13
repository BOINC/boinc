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
# Script to build the wxMac-3.0.0 wxCocoa library for BOINC
#
# by Charlie Fenton    7/21/06
# Updated for wx-Mac 2.8.10 and Unicode 4/17/09
# Updated for OS 10.7 and XCode 4.1 with OS 10.4 compatibility 9/26/11
# Updated for partial OS 10.8 and XCode 4.5 compatibility 7/27/12
# Updated for wxCocoa 2.9.5 8/20/13
# Updated for wxCocoa 3.0.0 11/12/13
# Fix for wxCocoa 3.0.0 2/13/14
#
## This script requires OS 10.6 or later
##
## In Terminal, CD to the wxWidgets-3.0.0 directory.
##    cd [path]/wxWidgets-3.0.0/
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

if [ "$1" != "-clean" ] && [ -f build/osx/build/Release/libwx_osx_cocoa_static.a ]; then
    echo "Release libwx_osx_cocoa_static.a already built"
else

##    export DEVELOPER_SDK_DIR="/Developer/SDKs"
    ## We must override some of the build settings in wxWindows.xcodeproj 
    xcodebuild -project build/osx/wxcocoa.xcodeproj -target static -configuration Release $doclean build ARCHS="i386" OTHER_CFLAGS="-Wall -Wundef -fno-strict-aliasing -fno-common -DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -DwxUSE_UNICODE=1 -DwxDEBUG_LEVEL=0 -fvisibility=hidden" OTHER_CPLUSPLUSFLAGS="-Wall -Wundef -fno-strict-aliasing -fno-common -DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -DwxUSE_UNICODE=1 -DwxDEBUG_LEVEL=0 -fvisibility=hidden -fvisibility-inlines-hidden" GCC_PREPROCESSOR_DEFINITIONS="WXBUILDING __WXOSX_COCOA__ __WX__ wxUSE_BASE=1 _FILE_OFFSET_BITS=64 _LARGE_FILES MACOS_CLASSIC __WXMAC_XCODE__=1 SCI_LEXER WX_PRECOMP=1 wxUSE_UNICODE_UTF8=1 wxUSE_UNICODE_WCHAR=0"

if [  $? -ne 0 ]; then return 1; fi
fi

if [ "$1" != "-clean" ] && [ -f build/osx/build/Debug/libwx_osx_cocoa_static.a ]; then
    echo "Debug libwx_osx_cocoa_static.a already built"
else
##    export DEVELOPER_SDK_DIR="/Developer/SDKs"
    ## We must override some of the build settings in wxWindows.xcodeproj 
    xcodebuild -project build/osx/wxcocoa.xcodeproj -target static -configuration Debug $doclean build ARCHS="i386" OTHER_CFLAGS="-Wall -Wundef -fno-strict-aliasing -fno-common -DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -DwxUSE_UNICODE=1 -DwxDEBUG_LEVEL=0 -fvisibility=hidden" OTHER_CPLUSPLUSFLAGS="-Wall -Wundef -fno-strict-aliasing -fno-common -DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -DwxUSE_UNICODE=1 -DwxUSE_UNICODE_UTF8=1 -DwxUSE_UNICODE_WCHAR=0 -fvisibility=hidden -fvisibility-inlines-hidden" GCC_PREPROCESSOR_DEFINITIONS="WXBUILDING __WXOSX_COCOA__ __WX__ wxUSE_BASE=1 _FILE_OFFSET_BITS=64 _LARGE_FILES MACOS_CLASSIC __WXMAC_XCODE__=1 SCI_LEXER WX_PRECOMP=1 wxUSE_UNICODE_UTF8=1 wxUSE_UNICODE_WCHAR=0"

if [  $? -ne 0 ]; then return 1; fi
fi

return 0

