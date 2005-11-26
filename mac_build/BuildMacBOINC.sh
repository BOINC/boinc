#!/bin/sh

##
# Script for building Macintosh BOINC Manager, Core Client and libraries
# by Charlie Fenton 10/12/05
# with thanks to Reinhard Prix for his assistance
##

## Usage:
## cd to the mac_build directory of the boinc tree, for example:
##     cd [path]/boinc/mac_build
##
## then invoke this script as follows:
##      source BuildMacBOINC.sh [-dev] [-noclean]
## or
##      chmod +x BuildMacBOINC.sh
##      ./BuildMacBOINC.sh [-dev] [-noclean]
##
## optional arguments
## -dev         build the development (debug) version. 
##              default is deployment (release) version.
##
## -noclean     don't do a "clean all" before building.
##              default is to clean all first.
##

if [ "$1" = "-dev" ] || [ "$2" = "-dev" ]; then
echo "Development (debug) build"
style="Development"
else
echo "Deployment (release) build"
style="Deployment"
fi

if [ "$1" = "-noclean" ] || [ "$2" = "-noclean" ]; then
doclean=""
else
echo "Clean all"
doclean="clean "
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
if [ -d /Developer/SDKs/MacOSX10.3.9.sdk/ ]; then
sdkname="/Developer/SDKs/MacOSX10.3.9.sdk"
else
echo "ERROR: System 10.3.9 SDK is missing.  For details, see build instructions at "
echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or http://boinc.berkeley.edu/mac_build.html"
exit 1
fi
elif [ "$major" = "7" ]; then
echo "Building BOINC under System 10.3"
sdkname=""
else
echo "ERROR: Building BOINC requires System 10.3 or later.  For details, see build instructions at "
echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or http://boinc.berkeley.edu/mac_build.html"
exit 1
fi

xcodebuild -project boinc.pbproj -target Build_All -buildstyle $style $doclean build NEXTROOT=$sdkname SDKROOT=$sdkname
