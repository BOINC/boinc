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
##
# Script for building Macintosh BOINC Manager, Core Client and libraries
# by Charlie Fenton 3/27/08
# with thanks to Reinhard Prix for his assistance
#
# Updated for OS 10.7 Lion and XCode 4.2 on 10/19/11
# Updated 7/9/12 for Xcode 4.3 and later which are not at a fixed address
# Updated 2/7/14 to also build libboinc_zip.a
# Updated 11/28/15 to build ScreenSaver with ARC under Xcode 6 or later
# Updated 2/15/16 to allow optional use of libc++ and C++11 dialect
# Updated 3/11/16 to remove obsolete targets MakeAppIcon_h & WaitPermissions
# Updated 3/13/16 to add -target and -setting optional arguments
# Updated 10/17/17 to fix bug when -all argument is implied but not explicitly passed
# Updated 10/19/17 Special handling of screensaver build is no longer needed
# Updated 10/14/18 for Xcode 10 (use this script only with BOINC 7.15 or later)
#
## This script requires OS 10.8 or later
#
## If you drag-install Xcode 4.3 or later, you must have opened Xcode
## and clicked the Install button on the dialog which appears to
## complete the Xcode installation before running this script.
##

## Usage:
## cd to the mac_build directory of the boinc tree, for example:
##     cd [path]/boinc/mac_build
##
## then invoke this script as follows:
##      source BuildMacBOINC.sh [-dev] [-noclean] [-libstdc++] [-c++11] [-all] [-lib] [-client] [-target targetName] [-setting name value] [-help]
## or
##      chmod +x BuildMacBOINC.sh
##      ./BuildMacBOINC.sh [-dev] [-noclean] [-libstdc++] [-c++11] [-all] [-lib] [-client] [-target targetName] [-setting name value] [-help]
##
## optional arguments
## -dev         build the development (debug) version.
##              default is deployment (release) version.
##
## -noclean     don't do a "clean" of each target before building.
##              default is to clean all first.
##
## -libstdc++   build using libstdc++ instead of libc++
##
## -c++11       build using c++11 language dialect instead of default (incompatible with libstdc++)
##
##  The following arguments determine which targets to build
##
## -all         build all targets (i.e. target "Build_All" -- this is the default)
##
## -lib         build the six libraries: libboinc_api.a, libboinc_graphics2.a,
##              libboinc.a, libboinc_opencl.a, libboinc_zip.a, jpeglib.a.
##
## -client      build two targets: boinc client and command-line utility boinc_cmd
##              (also builds libboinc.a if needed, since boinc_cmd requires it.)
##
## Both -lib and -client may be specified to build seven targets (no BOINC Manager)
##
## The following are used mainly for building the daily test builds:
##
## -target targetName build ONLY the one target specified by targetName
##
## -setting name value override setting 'name' to have the value 'value'
## Usually used along with -target, You can pass multipe -setting arguments.
##

targets=""
doclean="clean"
cplusplus11dialect=""
uselibcplusplus=""
buildall=0
buildlibs=0
buildclient=0
buildzip=1
style="Deployment"
unset settings

while [ $# -gt 0 ]; do
  case "$1" in
    -noclean ) doclean="" ; shift 1 ;;
    -dev ) style="Development" ; shift 1 ;;
    -libstdc++ ) uselibcplusplus="CLANG_CXX_LIBRARY=libstdc++" ; shift 1 ;;
    -c++11 ) cplusplus11dialect="CLANG_CXX_LANGUAGE_STANDARD=c++11" ; shift 1 ;;
    -all ) buildall=1 ; shift 1 ;;
    -lib ) buildlibs=1 ; shift 1 ;;
    -client ) buildclient=1 ; shift 1 ;;
    -target ) shift 1 ; targets="-target $1" ; buildzip=0 ; shift 1 ;;
    -setting ) shift 1 ; name="$1" ;
                shift 1 ; unset value ; value=("$1");
                settings+=("$name=""${value[@]}") ;
                shift 1 ;;
    * ) echo "usage:" ; echo "cd {path}/mac_build/" ; echo "source BuildMacBOINC.sh [-dev] [-noclean] [-all] [-lib] [-client]  [-target targetName] [-setting name value] [-help]" ; return 1 ;;
  esac
done

if [ "${doclean}" = "clean" ]; then
    echo "Clean each target before building"
fi

if [ "${buildlibs}" = "1" ]; then
    targets="$targets -target libboinc -target gfx2libboinc -target api_libboinc -target boinc_opencl -target jpeg"
fi

if [ "${buildclient}" = "1" ]; then
    targets="$targets -target BOINC_Client -target cmd_boinc"
fi

## "-all" overrides "-lib" and "-client" since it includes those targets
if [ "${buildall}" = "1" ] || [ "${targets}" = "" ]; then
    buildall=1
    targets="-target Build_All"
fi

version=`uname -r`;

major=`echo $version | sed 's/\([0-9]*\)[.].*/\1/' `;
# minor=`echo $version | sed 's/[0-9]*[.]\([0-9]*\).*/\1/' `;

# echo "major = $major"
# echo "minor = $minor"
#
# Darwin version 12.x.y corresponds to OS 10.8.x
# Darwin version 11.x.y corresponds to OS 10.7.x
# Darwin version 10.x.y corresponds to OS 10.6.x
# Darwin version 9.x.y corresponds to OS 10.5.x
# Darwin version 8.x.y corresponds to OS 10.4.x
# Darwin version 7.x.y corresponds to OS 10.3.x
# Darwin version 6.x corresponds to OS 10.2.x

if [ "$major" -lt "11" ]; then
    echo "ERROR: Building BOINC requires System 10.7 or later.  For details, see build instructions at"
    echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or http://boinc.berkeley.edu/trac/wiki/MacBuild"
    return 1
fi

if [ "${style}" = "Development" ]; then
    echo "Development (debug) build"
else
    style="Deployment"
    echo "Deployment (release) build for architecture: x86_64"
fi

echo ""

SDKPATH=`xcodebuild -version -sdk macosx Path`
result=0

xcodebuild -project boinc.xcodeproj ${targets} -configuration ${style} -sdk "${SDKPATH}" ${doclean} build ${uselibcplusplus} ${cplusplus11dialect} "${settings[@]}"
result=$?

if [ $result -eq 0 ]; then
    # build ibboinc_zip.a for -all or -lib or default, where
    # default is none of { -all, -lib, -client }
    if [ "${buildall}" = "1" ] || [ "${buildlibs}" = "1" ] || [ "${buildclient}" = "0" ]; then
        if [ "${buildzip}" = "1" ]; then
            xcodebuild -project ../zip/boinc_zip.xcodeproj -target boinc_zip -configuration ${style} -sdk "${SDKPATH}" ${doclean} build  ${uselibcplusplus} ${cplusplus11dialect}
            result=$?
        fi
    fi
fi

return $result
