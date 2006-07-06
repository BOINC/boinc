#!/bin/csh

## Berkeley Open Infrastructure for Network Computing
## http://boinc.berkeley.edu
## Copyright (C) 2005 University of California
##
## This is free software; you can redistribute it and/or
## modify it under the terms of the GNU Lesser General Public
## License as published by the Free Software Foundation;
## either version 2.1 of the License, or (at your option) any later version.
##
## This software is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU Lesser General Public License for more details.
##
## To view the GNU Lesser General Public License visit
## http://www.gnu.org/copyleft/lesser.html
## or write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

##
# Release Script for Macintosh BOINC Manager 2/16/06 by Charlie Fenton
##

## Usage:
## cd to the root directory of the boinc tree, for example:
##     cd [path]/boinc
##
## Invoke this script with the three parts of version number as arguments.  
## For example, if the version is 3.2.1:
##     source [path_to_this_script] 3 2 1
##
## This will create a director "BOINC_Installer" in the parent directory of 
## the current directory
##
## For testing only, you can use the development build by adding a fourth argument -dev
## For example, if the version is 3.2.1:
##     source [path_to_this_script] 3 2 1 -dev

if [ $# -lt 3 ]; then
echo "Usage:"
echo "   cd [path]/boinc"
echo "   source [path_to_this_script] major_version minor_version revision_number"
return 1
fi

pushd ./

## XCode 2.x has separate directories for Development and Deployment build products
if [ "$4" = "-dev" ]; then
    if [ -d mac_build/build/Development/ ]; then
        BUILDPATH="mac_build/build/Development"
    else
        BUILDPATH="mac_build/build"
    fi
else
    if [ -d mac_build/build/Deployment/ ]; then
        BUILDPATH="mac_build/build/Deployment"
    else
        BUILDPATH="mac_build/build"
    fi
fi

sudo rm -dfR ../BOINC_Installer/Installer\ Resources/
sudo rm -dfR ../BOINC_Installer/Pkg_Root

mkdir -p ../BOINC_Installer/Installer\ Resources/

cp -fp mac_Installer/License.rtf ../BOINC_Installer/Installer\ Resources/
cp -fp mac_installer/ReadMe.rtf ../BOINC_Installer/Installer\ Resources/
# Update version number
sed -i "" s/"<VER_NUM>"/"$1.$2.$3"/g ../BOINC_Installer/Installer\ Resources/ReadMe.rtf

#### We don't customize BOINC Data directory name for branding
#### cp -fp mac_installer/preinstall ../BOINC_Installer/Installer\ Resources/
#### cp -fp mac_installer/preinstall ../BOINC_Installer/Installer\ Resources/preupgrade
cp -fp mac_installer/postinstall ../BOINC_Installer/Installer\ Resources/
cp -fp mac_installer/postupgrade ../BOINC_Installer/Installer\ Resources/

cp -fpR $BUILDPATH/Postinstall.app ../BOINC_Installer/Installer\ Resources/

mkdir -p ../BOINC_Installer/Pkg_Root
mkdir -p ../BOINC_Installer/Pkg_Root/Applications
mkdir -p ../BOINC_Installer/Pkg_Root/Library
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Screen\ Savers
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/locale
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/switcher

cp -fpR $BUILDPATH/switcher ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/switcher/
cp -fpR $BUILDPATH/setprojectgrp ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/switcher/

cp -fpR $BUILDPATH/BOINCManager.app ../BOINC_Installer/Pkg_Root/Applications/

cp -fpR $BUILDPATH/BOINCSaver.saver ../BOINC_Installer/Pkg_Root/Library/Screen\ Savers/

## Copy the localization files into the installer tree
## Old way copies CVS and *.po files which are not needed
## cp -fpR locale/client/ ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/locale
## sudo rm -dfR ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/locale/CVS
## New way copies only *.mo files (adapted from boinc/sea/make-tar.sh)
find locale/client -name '*.mo' | cut -d '/' -f 3 | awk '{print "\"../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/locale/"$0"\""}' | xargs mkdir -p
find locale/client -name '*.mo' | cut -d '/' -f 3,4 | awk '{print "cp \"locale/client/"$0"\" \"../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/locale/"$0"\""}' | bash

## Fix up ownership and permissions
sudo chown -R root:admin ../BOINC_Installer/Pkg_Root/*
sudo chmod -R u+rw,g+rw,o+r-w ../BOINC_Installer/Pkg_Root/*
sudo chmod 1775 ../BOINC_Installer/Pkg_Root/Library

sudo chown -R 501:admin ../BOINC_Installer/Pkg_Root/Library/Application\ Support/*
sudo chmod -R u+rw,g+r-w,o+r-w ../BOINC_Installer/Pkg_Root/Library/Application\ Support/*

sudo chown -R root:admin ../BOINC_Installer/Installer\ Resources/*
sudo chmod -R u+rw,g+r-w,o+r-w ../BOINC_Installer/Installer\ Resources/*

sudo rm -dfR ../BOINC_Installer/New_Release_$1_$2_$3/

mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/
mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_universal
mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_universal-apple-darwin
mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_SymbolTables

cp -fp ../BOINC_Installer/Installer\ Resources/ReadMe.rtf ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_universal
sudo chown -R 501:admin ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_universal/ReadMe.rtf
sudo chmod -R 644 ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_universal/ReadMe.rtf

cp -fpR $BUILDPATH/boinc ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_universal-apple-darwin/
cp -fpR $BUILDPATH/boinc_cmd ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_universal-apple-darwin/
sudo chown -R root:admin ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_universal-apple-darwin/*
sudo chmod -R u+rw,g+r-w,o+r-w ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_universal-apple-darwin/*

cp -fpR $BUILDPATH/SymbolTables ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_SymbolTables/

# Copy the installer wrapper application "BOINC Installer.app"
cp -fpR $BUILDPATH/BOINC\ Installer.app ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_universal/
# Build the installer package inside the wrapper application's bundle
/Developer/Tools/packagemaker -build -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_universal/BOINC\ Installer.app/Contents/Resources/BOINC.pkg -f ../BOINC_Installer/Pkg_Root -r ../BOINC_Installer/Installer\ Resources/ -i mac_build/Pkg-Info.plist -d mac_Installer/Description.plist -ds 
# Allow the installer wrapper application to modify the package's Info.plist file
sudo chmod u+w,g+w,o+w ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_universal/BOINC\ Installer.app/Contents/Resources/BOINC.pkg/Contents/Info.plist

cd ../BOINC_Installer/New_Release_$1_$2_$3
zip -rqy boinc_$1.$2.$3_macOSX_universal.zip boinc_$1.$2.$3_macOSX_universal
zip -rqy boinc_$1.$2.$3_universal-apple-darwin.zip boinc_$1.$2.$3_universal-apple-darwin
zip -rqy boinc_$1.$2.$3_macOSX_SymbolTables.zip boinc_$1.$2.$3_macOSX_SymbolTables

popd
return 0
