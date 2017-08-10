#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2017 University of California
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
# Patch to fix crash on OS 10.5 or 10.6 when built on OS 10.7+ 2/18/14
# Enable wxWidgets asserts in Release build 3/6/14
# Disable all wxWidgets debug support in Release build (revert 3/6/14 change) 5/29/14
# Fix wxListCtrl flicker when resizing columns in wxCocoa 3.0.0 6/13/14
# Revise fix for wxListCtrl flicker to match the fix in wxWidgets trunk 6/19/14
#
## This script requires OS 10.6 or later
##
## In Terminal, CD to the wxWidgets-3.0.0 directory.
##    cd [path]/wxWidgets-3.0.0/
## then run this script:
##    source [ path_to_this_script ] [ -clean ] [ -nodebug ] [--prefix PATH]
##
## the -clean argument will force a full rebuild.
## the -nodebug argument will ommit building the debug version of the library
## if --prefix is given as absolute path the library is installed into there
## use -q or --quiet to redirect build output to /dev/null instead of /dev/stdout
#

SRCDIR=$PWD
echo "${SRCDIR}" | grep " " > /dev/null 2>&1
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

echo ""

# Patch wxWidgets-3.0.0/src/png/pngstruct.h
if [ ! -f src/png/pngstruct.h.orig ]; then
    cat >> /tmp/pngstruct_h_diff << ENDOFFILE
--- pngstruct.h	2013-11-11 05:10:39.000000000 -0800
+++ pngstruct_patched.h	2014-02-18 01:31:53.000000000 -0800
@@ -34,6 +34,13 @@
 #  undef const
 #endif

+/* BOINC workaround patch to fix crashes on OS 10.5 or 10.6 when
+ * built with OS 10.7 SDK or later.
+ */
+#undef ZLIB_VERNUM
+#define ZLIB_VERNUM 0x1230
+/* End of BOINC workaround patch */
+
 /* zlib.h has mediocre z_const use before 1.2.6, this stuff is for compatibility
  * with older builds.
  */
ENDOFFILE
    patch -bfi /tmp/pngstruct_h_diff src/png/pngstruct.h
    rm -f /tmp/pngstruct_h_diff
else
    echo "src/png/pngstruct.h already patched"
fi

echo ""

# Patch build/osx/setup/cocoa/include/wx/setup.h
if [ ! -f build/osx/setup/cocoa/include/wx/setup.h.orig ]; then

# First run wxWidget's built-in script to copy setup.h into place
    cd build/osx || return 1
    ../../distrib/mac/pbsetup-sh ../../src ../../build/osx/setup/cocoa
    cd ../.. || return 1

    cat >> /tmp/setup_h_diff << ENDOFFILE
--- setup.h	2014-02-18 05:17:45.000000000 -0800
+++ setup_patched.h	2014-02-18 05:19:50.000000000 -0800
@@ -339,7 +339,10 @@
 // Recommended setting: 1 if you use the standard streams anyhow and so
 //                      dependency on the standard streams library is not a
 //                      problem
-#define wxUSE_STD_IOSTREAM  wxUSE_STD_DEFAULT
+/* BOINC workaround patch to fix crashes on OS 10.5 when built
+ * with OS 10.7 SDK or later.
+ */
+#define wxUSE_STD_IOSTREAM 0 // wxUSE_STD_DEFAULT

 // Enable minimal interoperability with the standard C++ string class if 1.
 // "Minimal" means that wxString can be constructed from std::string or
ENDOFFILE
    patch -bfi /tmp/setup_h_diff build/osx/setup/cocoa/include/wx/setup.h
    rm -f /tmp/setup_h_diff
else
    echo "build/osx/setup/cocoa/include/wx/setup.h already patched"
fi

echo ""

# Patch wxWidgets-3.0.0/src/osx/carbon/dcclient.cpp to eliminate flicker when resizing columns
if [ ! -f src/osx/carbon/dcclient.cpp.orig ]; then
    cat >> /tmp/listctrl_cpp_diff << ENDOFFILE
--- src/osx/carbon/dcclient.cpp	2014-06-12 22:15:31.000000000 -0700
+++ src/osx/carbon/dcclient-patched.cpp 2014-06-19 01:04:58.000000000 -0700
@@ -174,7 +174,7 @@

 wxClientDCImpl::~wxClientDCImpl()
 {
-    if( GetGraphicsContext() && GetGraphicsContext()->GetNativeContext() )
+if( GetGraphicsContext() && GetGraphicsContext()->GetNativeContext() && !m_release )
         Flush();
 }
ENDOFFILE
    patch -bfi /tmp/listctrl_cpp_diff src/osx/carbon/dcclient.cpp
    rm -f /tmp/listctrl_cpp_diff
else
    echo "src/osx/carbon/dcclient.cpp already patched"
fi

echo ""

doclean=""
stdout_target="/dev/stdout"
lprefix=""
libPathRel="build/osx/build/Release"
libPathDbg="build/osx/build/Debug"
nodebug=""
beautifier="cat" # we need a fallback if xcpretty is not available
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -clean|--clean)
        doclean="clean"
        ;;
        -nodebug|--nodebug)
        nodebug="yes"
        ;;
        -prefix|--prefix)
        lprefix="$2"
        libPathRel="${lprefix}/lib"
        libPathDbg="${lprefix}/lib/debug"
        shift
        ;;
        -q|--quiet)
        stdout_target="/dev/null"
        ;;
    esac
    shift # past argument or value
done

XCPRETTYPATH=`xcrun -find xcpretty 2>/dev/null`
if [ $? -eq 0 ]; then
    beautifier="xcpretty"
fi

retval=0

if [ "${doclean}" != "clean" ] && [ -f "${libPathRel}/libwx_osx_cocoa_static.a" ]; then
    cwd=$(pwd)
    dirname=${cwd##*/}
    echo "${dirname} Release libwx_osx_cocoa_static.a already built"
else

##    export DEVELOPER_SDK_DIR="/Developer/SDKs"
    ## We must override some of the build settings in wxWindows.xcodeproj
    xcodebuild -project build/osx/wxcocoa.xcodeproj -target static -configuration Release $doclean build ARCHS="i386" OTHER_CFLAGS="-Wall -Wundef -fno-strict-aliasing -fno-common -DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -DwxUSE_UNICODE=1 -DwxDEBUG_LEVEL=0 -DNDEBUG -fvisibility=hidden" OTHER_CPLUSPLUSFLAGS="-Wall -Wundef -fno-strict-aliasing -fno-common -DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -DwxUSE_UNICODE=1 -DwxDEBUG_LEVEL=0 -DNDEBUG -fvisibility=hidden -fvisibility-inlines-hidden" GCC_PREPROCESSOR_DEFINITIONS="WXBUILDING __WXOSX_COCOA__ __WX__ wxUSE_BASE=1 _FILE_OFFSET_BITS=64 _LARGE_FILES MACOS_CLASSIC __WXMAC_XCODE__=1 SCI_LEXER WX_PRECOMP=1 wxUSE_UNICODE_UTF8=1 wxUSE_UNICODE_WCHAR=0" | $beautifier; retval=${PIPESTATUS[0]}
    if [ ${retval} -ne 0 ]; then return 1; fi
    if [ "x${lprefix}" != "x" ]; then
        # copy library and headers to $lprefix
        mkdir -p "${libPathRel}"
        mkdir -p "${lprefix}/include"
        cp build/osx/build/Release/libwx_osx_cocoa_static.a "${libPathRel}"
        strip -x "${libPathRel}/libwx_osx_cocoa_static.a"
        cp -R include/wx "${lprefix}/include"
        cp build/osx/setup/cocoa/include/wx/setup.h "${lprefix}/include/wx"
    fi
fi

if [ "${nodebug}" = "yes" ]; then
    return 0
fi

if [ "${doclean}" != "clean" ] && [ -f "${libPathDbg}/libwx_osx_cocoa_static.a" ]; then
    cwd=$(pwd)
    dirname=${cwd##*/}
    echo "${dirname} Debug libwx_osx_cocoa_static.a already built"
else
##    export DEVELOPER_SDK_DIR="/Developer/SDKs"
    ## We must override some of the build settings in wxWindows.xcodeproj
    xcodebuild -project build/osx/wxcocoa.xcodeproj -target static -configuration Debug $doclean build ARCHS="i386" OTHER_CFLAGS="-Wall -Wundef -fno-strict-aliasing -fno-common -DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -DwxUSE_UNICODE=1 -DDEBUG -fvisibility=hidden" OTHER_CPLUSPLUSFLAGS="-Wall -Wundef -fno-strict-aliasing -fno-common -DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -DwxUSE_UNICODE=1 -DDEBUG -fvisibility=hidden -fvisibility-inlines-hidden" GCC_PREPROCESSOR_DEFINITIONS="WXBUILDING __WXOSX_COCOA__ __WX__ wxUSE_BASE=1 _FILE_OFFSET_BITS=64 _LARGE_FILES MACOS_CLASSIC __WXMAC_XCODE__=1 SCI_LEXER WX_PRECOMP=1 wxUSE_UNICODE_UTF8=1 wxUSE_UNICODE_WCHAR=0" | $beautifier; retval=${PIPESTATUS[0]}
    if [ ${retval} -ne 0 ]; then return 1; fi
    if [ "x${lprefix}" != "x" ]; then
        # copy debug library to $PREFIX
        mkdir -p "${libPathDbg}"
        cp build/osx/build/Debug/libwx_osx_cocoa_static.a "${libPathDbg}"
    fi
fi
return 0
