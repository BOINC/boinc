#!/bin/sh

##
# Script for building Macintosh BOINC Manager, Core Client and libraries
# by Charlie Fenton 2/8/06
# with thanks to Reinhard Prix for his assistance
##

## Usage:
## cd to the mac_build directory of the boinc tree, for example:
##     cd [path]/boinc/mac_build
##
## then invoke this script as follows:
##      source BuildMacBOINC.sh [-dev] [-noclean] [-all] [-lib] [-client] [-help]
## or
##      chmod +x BuildMacBOINC.sh
##      ./BuildMacBOINC.sh [-dev] [-noclean] [-all] [-lib] [-client]
##
## optional arguments
## -dev         build the development (debug) version (native architecture only). 
##              default is deployment (release) version (universal binaries: ppc and i386).
##
## -noclean     don't do a "clean" of each target before building.
##              default is to clean all first.
##
##  The following arguments determine which targets to build
##
## -all         build all targets (i.e. target "Build_All" -- this is the default)
##
## -lib         build the three libraries: libboinc_api.a, libboinc_graphics_api.a, libboinc.a
##
## -client      build two targets: boinc client and command-line utility boinc_cmd
##              (also builds libboinc.a if needed, since boinc_cmd requires it.)
##
## Both -lib and -client may be specified to build five targets (no BOINC Manager)
##

targets=""
style="Deployment"
doclean="clean"
buildall=0
buildlibs=0
buildclient=0

while [ $# -gt 0 ]; do
  case "$1" in 
    -noclean ) doclean="" ; shift 1 ;;
    -dev ) style="Development" ; shift 1 ;;
    -all ) buildall=1 ; shift 1 ;;
    -lib ) buildlibs=1 ; shift 1 ;;
    -client ) buildclient=1 ; shift 1 ;;
    * ) echo "usage:" ; echo "cd {path}/mac_build/" ; echo "source BuildMacBOINC.sh [-dev] [-noclean] [-all] [-lib] [-client] [-help]" ; return 1 ;;
  esac
done

if [ "${style}" = "Development" ]; then
    echo "Development (debug) build"
else
    echo "Deployment (release) build"
fi

if [ "${doclean}" = "clean" ]; then
    echo "Clean each target before building"
fi

if [ "${buildlibs}" = "1" ]; then
targets="$targets -target libboinc -target gfxlibboinc -target api_libboinc"
fi

if [ "${buildclient}" = "1" ]; then
targets="$targets -target BOINC_Client -target cmd_boinc"
fi

## "-all" overrides "-lib" and "-client" since it includes those targets
if [ "${buildall}" = "1" ] || [ "${targets}" = "" ]; then
    targets="-target Build_All"
fi

version=`uname -r`;

major=`echo $version | sed 's/\([0-9]*\)[.].*/\1/' `;
minor=`echo $version | sed 's/[0-9]*[.]\([0-9]*\).*/\1/' `;

# echo "major = $major"
# echo "minor = $minor"
#
# Darwin version 8.x.y corresponds to OS 10.4.x
# Darwin version 7.x.y corresponds to OS 10.3.x
# Darwin version 6.x corresponds to IS 10.2.x

if [ "$major" = "8" ]; then
echo "Building BOINC under System 10.4"
if [ ! -d /Developer/SDKs/MacOSX10.3.9.sdk/ ]; then
    echo "ERROR: System 10.3.9 SDK is missing.  For details, see build instructions at "
    echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or http://boinc.berkeley.edu/mac_build.html"
    return 1
fi
else
    echo "ERROR: Building BOINC requires System 10.4 or later.  For details, see build instructions at "
    echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or http://boinc.berkeley.edu/mac_build.html"
    return 1
fi

xcodebuild -project boinc.xcodeproj ${targets} -configuration ${style} ${doclean} build

return $?
