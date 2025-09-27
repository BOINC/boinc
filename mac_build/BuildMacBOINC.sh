#!/bin/bash

# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2025 University of California
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
# Updated 3/31/21 To eliminate redundant -c++11 arg since C++11 build is now standard
# Updated 5/19/21 for compatibility with zsh
# Updated 7/12/22 result is moved out of eval string to get correct status on CI if build fails
# Updated 2/14/23 refactoring made to build zip apps (-zipapps), uc2 samples (-uc2) and vboxwrapper (-vboxwrapper)
# Updated 3/12/23 Don't unnecessary rebuild libraries for uc2, zip apps or vboxwrapper
# Updated 3/29/25 Build docker_wrapper
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
##      source BuildMacBOINC.sh [-dev] [-noclean] [-libstdc++] [-all] [-lib] [-client] [-uc2] [-vboxwrapper] [-docker_wrapper] [-target targetName] [-setting name value] [-help]
## or
##      chmod +x BuildMacBOINC.sh
##      ./BuildMacBOINC.sh [-dev] [-noclean] [-libstdc++] [-all] [-lib] [-client] [-uc2] [-vboxwrapper] [-docker_wrapper] [-target targetName] [-setting name value] [-help]
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
##
##  The following arguments determine which targets to build
##
## -all         build all targets (i.e. target "Build_All" -- this is the default)
##              except UpperCase2 targets (UC2-x86_64, UC2Gfx-x86_64 and
##              slide_show-x86_64) and VBoxWrapper
##
## -lib         build the six libraries: libboinc_api.a, libboinc_graphics2.a,
##              libboinc.a, libboinc_opencl.a, libboinc_zip.a, jpeglib.a.
##
## -client      build two targets: boinc client and command-line utility boinc_cmd
##              (also builds libboinc.a if needed, since boinc_cmd requires it.)
##
## -uc2         build the UpperCase2 targets: UC2-x86_64, UC2Gfx-x86_64 and
##              slide_show-x86_64
##
## -vboxwrapper build the VBoxWrapper target
##
## -docker_wrapper build the docker_wrapper target
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
uselibcplusplus=""
buildall=0
buildlibs=0
buildclient=0
buildzip=0
builduc2=0
buildvboxwrapper=0
builddocker_wrapper=0
style="Deployment"
unset settings

while [ $# -gt 0 ]; do
  case "$1" in
    -noclean ) doclean="" ; shift 1 ;;
    -dev ) style="Development" ; shift 1 ;;
    -libstdc++ ) uselibcplusplus="CLANG_CXX_LIBRARY=libstdc++" ; shift 1 ;;
    -all ) buildall=1 ; shift 1 ;;
    -lib ) buildlibs=1 ; shift 1 ;;
    -client ) buildclient=1 ; shift 1 ;;
    -uc2 ) builduc2=1 ; shift 1 ;;
    -vboxwrapper ) buildvboxwrapper=1 ; shift 1 ;;
    -docker_wrapper ) builddocker_wrapper=1 ; shift 1 ;;
    -target ) shift 1 ; targets="$targets -target $1" ; shift 1 ;;
    -setting ) shift 1 ; name="$1" ;
                shift 1 ; unset value ; value=("$1");
                settings+=("$name=""\"${value[@]}\"") ;
                shift 1 ;;
    * ) echo "usage:" ; echo "cd {path}/mac_build/" ; echo "source BuildMacBOINC.sh [-dev] [-noclean] [-libstdc++] [-all] [-lib] [-client] [-uc2] [-vboxwrapper] [-docker_wrapper] [-target targetName] [-setting name value] [-help]" ; return 1 ;;
  esac
done

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
    echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or https://github.com/BOINC/boinc/wiki/MacBuild"
    return 1
fi

if [ "${style}" = "Development" ]; then
    echo "Development (debug) build"
else
    style="Deployment"
    echo "Deployment (release) build for architecture: x86_64"
fi

echo ""

if [ "${builduc2}" = "1" ]; then
    if [ ! -e "./build/${style}/libboinc.a" ]; then buildlibs=1; fi
    if [ ! -e "./build/${style}/libboinc_api.a" ]; then buildlibs=1; fi
    if [ ! -e "./build/${style}/libboinc_api.a" ]; then buildlibs=1; fi
    if [ ! -e "./build/${style}/libjpeg.a" ]; then buildlibs=1; fi
    if [ ! -e "./build/${style}/libboinc_zip.a" ]; then buildzip=1; fi
fi

if [ "${buildvboxwrapper}" = "1" ] || [ "${builddocker_wrapper}" = "1" ]; then
    if [ ! -e "./build/${style}/libboinc.a" ]; then buildlibs=1; fi
    if [ ! -e "./build/${style}/libboinc_api.a" ]; then buildlibs=1; fi
fi

if [ "${doclean}" = "clean" ]; then
    echo "Clean each target before building"
fi

if [ "${buildlibs}" = "1" ]; then
    targets="$targets -target libboinc -target gfx2libboinc -target api_libboinc -target boinc_opencl -target jpeg"
fi

if [ "${buildclient}" = "1" ]; then
    targets="$targets -target BOINC_Client -target cmd_boinc"
fi

if [ "x${targets}" = "x" ] && [ "${buildlibs}" = "0" ] && [ "${buildclient}" = "0" ] && [ "${builduc2}" = "0" ] && [ "${buildvboxwrapper}" = "0" ] && [ "${builddocker_wrapper}" = "0" ] ; then
    buildall=1
fi

## "-all" overrides "-lib" and "-client" and "-uc2" and "-vboxwrapper" and "-docker_wrapper" since it includes those targets
if [ "${buildall}" = "1" ]; then
    targets="-target Build_All"
fi

SDKPATH=`xcodebuild -version -sdk macosx Path`
result=0

## Passing "${settings[@]}" to xcodebuild generates an error under zsh if
## the settings array is empty, so we build a single string from the array.
theSettings=""
for f in "${settings[@]}"; do
    theSettings+="${f}"
    theSettings+=" "
done

## For unknown reasons, this xcodebuild call generates syntax errors under zsh
## unless we enclose the command in quotes and invoke it with eval.
## That is why all the other xcodebuild calls are invoked this way.

if [ "${buildall}" = "1" ] || [ "${buildlibs}" = "1" ] || [ "${buildclient}" = "1" ] || [ "x${targets}" != "x" ]; then

    echo ""
    ## Apparently xcodebuild ignores build pre-actions, so we do this explicitly
    source "./Update_Info_Plists.sh"
    result=$?
    echo ""

    if [ $result -eq 0 ]; then
        # build all or specified targets from the boinc.xcodeproj project for -all, -libs, -client, or -target
        eval "xcodebuild -project boinc.xcodeproj ${targets} -configuration ${style} -sdk \"${SDKPATH}\" ${doclean} build ${uselibcplusplus} ${theSettings}"
        result=$?
    fi
fi

if [ $result -eq 0 ]; then
    # build libboinc_zip.a for -all or -lib
    if [ "${buildall}" = "1" ] || [ "${buildlibs}" = "1" ]; then
        buildzip=1
    fi

    if [ "${buildzip}" = "1" ]; then
        eval "xcodebuild -project ../zip/boinc_zip.xcodeproj -target boinc_zip -configuration ${style} -sdk \"${SDKPATH}\" ${doclean} build  ${uselibcplusplus} ${theSettings}"
        result=$?
    fi
fi

if [ $result -eq 0 ]; then
    # build UC2 sample apps for -uc2
    if [ "${builduc2}" = "1" ]; then
        eval "xcodebuild -project ../samples/mac_build/UpperCase2.xcodeproj -target Build_All -configuration ${style} -sdk \"${SDKPATH}\" ${doclean} build  ${uselibcplusplus} ${theSettings}"
        result=$?
    fi
fi

if [ $result -eq 0 ]; then
    # build vboxwrapper app for -vboxwrapper
    if [ "${buildvboxwrapper}" = "1" ]; then
        eval "xcodebuild -project ../samples/vboxwrapper/vboxwrapper.xcodeproj -target Build_All -configuration ${style} -sdk \"${SDKPATH}\" ${doclean} build  ${uselibcplusplus} ${theSettings}"
        result=$?
    fi
fi

if [ $result -eq 0 ]; then
    # build docker_wrapper app for -docker_wrapper
    if [ "${builddocker_wrapper}" = "1" ]; then
        eval "xcodebuild -project ../samples/docker_wrapper/docker_wrapper.xcodeproj -target docker_wrapper -configuration ${style} -sdk \"${SDKPATH}\" ${doclean} build  ${uselibcplusplus} ${theSettings}"
        result=$?
    fi
fi

return $result
