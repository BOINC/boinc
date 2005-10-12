#!/bin/sh

##
# Script for building Macintosh BOINC Manager, Core Client and libraries
# by Charlie Fenton 9/29/05
##

## Usage:
## cd to the mac_build directory of the boinc tree, for example:
##     cd [path]/boinc/mac_build
##
## then invoke this script as follows:
##
## To build the deployment (release) version:
##      source BuildMacBOINC.sh [-dev] [-noclean]
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

if [ -d /Developer/SDKs/MacOSX10.3.9.sdk/ ]; then
echo "Building BOINC under System 10.4"
sdkname="/Developer/SDKs/MacOSX10.3.9.sdk"
elif [ -d /Developer/SDKs/MacOSX10.3.0.sdk/ ]; then
echo "Building BOINC under System 10.3"
sdkname="/Developer/SDKs/MacOSX10.3.0.sdk"
else
echo "ERROR: System 10.3 SDK is missing.  For details, see build instructions at "
echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or http://boinc.berkeley.edu/mac_build.html"
exit 1
fi

xcodebuild -project boinc.pbproj -target Build_All -buildstyle $style $doclean build NEXTROOT=$sdkname SDKROOT=$sdkname
