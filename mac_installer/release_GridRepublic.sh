#!/bin/csh

##
# Release Script for Macintosh BOINC Manager 1/21/06 by Charlie Fenton
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

if [ $# -lt 3 ]; then
echo "Usage:"
echo "   cd [path]/boinc"
echo "   source [path_to_this_script] major_version minor_version revision_number"
exit 0
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

sudo rm -dfR ../BOINC_Installer/GR_Installer\ Resources/
sudo rm -dfR ../BOINC_Installer/GR_Pkg_Root

mkdir -p ../BOINC_Installer/GR_Installer\ Resources/

cp -fp mac_Installer/License.rtf ../BOINC_Installer/GR_Installer\ Resources/
cp -fp mac_installer/GR-ReadMe.rtf ../BOINC_Installer/GR_Installer\ Resources/ReadMe.rtf
cp -fp mac_installer/GR-preinstall ../BOINC_Installer/GR_Installer\ Resources/preinstall
cp -fp mac_installer/GR-preupgrade ../BOINC_Installer/GR_Installer\ Resources/preupgrade
cp -fp mac_installer/postinstall ../BOINC_Installer/GR_Installer\ Resources/
cp -fp mac_installer/postupgrade ../BOINC_Installer/GR_Installer\ Resources/

cp -fpR $BUILDPATH/Postinstall.app ../BOINC_Installer/GR_Installer\ Resources/

mkdir -p ../BOINC_Installer/GR_Pkg_Root
mkdir -p ../BOINC_Installer/GR_Pkg_Root/Applications
mkdir -p ../BOINC_Installer/GR_Pkg_Root/Library
mkdir -p ../BOINC_Installer/GR_Pkg_Root/Library/Screen\ Savers
mkdir -p ../BOINC_Installer/GR_Pkg_Root/Library/Application\ Support
mkdir -p ../BOINC_Installer/GR_Pkg_Root/Library/Application\ Support/GridRepublic\ Data
mkdir -p ../BOINC_Installer/GR_Pkg_Root/Library/Application\ Support/GridRepublic\ Data/locale

cp -fpR $BUILDPATH/BOINCManager.app ../BOINC_Installer/GR_Pkg_Root/Applications/

cp -fpR $BUILDPATH/BOINCSaver.saver ../BOINC_Installer/GR_Pkg_Root/Library/Screen\ Savers/

## Copy the localization files into the installer tree
## Old way copies CVS and *.po files which are not needed
## cp -fpR locale/client/ ../BOINC_Installer/GR_Pkg_Root/Library/Application\ Support/BOINC\ Data/locale
## sudo rm -dfR ../BOINC_Installer/GR_Pkg_Root/Library/Application\ Support/BOINC\ Data/locale/CVS
## New way copies only *.mo files (adapted from boinc/sea/make-tar.sh)
find locale/client -name '*.mo' | cut -d '/' -f 3 | awk '{print "-p \"../BOINC_Installer/GR_Pkg_Root/Library/Application Support/GridRepublic\ Data/locale/"$0"\""}' | xargs mkdir
find locale/client -name '*.mo' | cut -d '/' -f 3,4 | awk '{print "cp \"locale/client/"$0"\" \"../BOINC_Installer/GR_Pkg_Root/Library/Application Support/GridRepublic\ Data/locale/"$0"\""}' | bash

## Modify for Grid Republic
# Rename the Manager's bundle and its executable inside the bundle
mv -f ../BOINC_Installer/GR_Pkg_Root/Applications/BOINCManager.app/ ../BOINC_Installer/GR_Pkg_Root/Applications/GridRepublic.app/
mv -f ../BOINC_Installer/GR_Pkg_Root/Applications/GridRepublic.app/Contents/MacOS/BOINCManager ../BOINC_Installer/GR_Pkg_Root/Applications/GridRepublic.app/Contents/MacOS/GridRepublic

# Replace the Manager's info.plist, InfoPlist.strings, BOINCMgr.icns
cp -fp mac_build/GR_Info.plist ../BOINC_Installer/GR_Pkg_Root/Applications/GridRepublic.app/Contents/Info.plist
cp -fp mac_build/GR-InfoPlist.strings ../BOINC_Installer/GR_Pkg_Root/Applications/GridRepublic.app/Contents/Resources/English.lproj/InfoPlist.strings
cp -fp client/mac/GridRepublic.icns ../BOINC_Installer/GR_Pkg_Root/Applications/GridRepublic.app/Contents/Resources/GridRepublic.icns
rm -f ../BOINC_Installer/GR_Pkg_Root/Applications/GridRepublic.app/Contents/Resources/BOINCMgr.icns

# Copy Branding file into both Application Bundle and Installer Package
cp -fp mac_installer/GR-Branding ../BOINC_Installer/GR_Pkg_Root/Applications/GridRepublic.app/Contents/Resources/Branding
cp -fp mac_installer/GR-Branding ../BOINC_Installer/GR_Installer\ Resources/Branding

# Rename the screensaver bundle and its executable inside the bundle
mv -f ../BOINC_Installer/GR_Pkg_Root/Library/Screen\ Savers/BOINCSaver.saver ../BOINC_Installer/GR_Pkg_Root/Library/Screen\ Savers/GridRepublic.saver
mv -f ../BOINC_Installer/GR_Pkg_Root/Library/Screen\ Savers/GridRepublic.saver/Contents/MacOS/BOINCSaver ../BOINC_Installer/GR_Pkg_Root/Library/Screen\ Savers/GridRepublic.saver/Contents/MacOS/GridRepublic

# Replace screensaver's info.plist, InfoPlist.strings, boinc.tif
cp -fp mac_build/GR-ScreenSaver-Info.plist ../BOINC_Installer/GR_Pkg_Root/Library/Screen\ Savers/GridRepublic.saver/Contents/Info.plist
cp -fp mac_build/GR-InfoPlist.strings ../BOINC_Installer/GR_Pkg_Root/Library/Screen\ Savers/GridRepublic.saver/Contents/Resources/English.lproj/InfoPlist.strings
cp -fp clientgui/mac/gridrepublic.tiff ../BOINC_Installer/GR_Pkg_Root/Library/Screen\ Savers/GridRepublic.saver/Contents/Resources/boinc.tiff

## Fix up ownership and permissions
sudo chown -R root:admin ../BOINC_Installer/GR_Pkg_Root/*
sudo chmod -R 775 ../BOINC_Installer/GR_Pkg_Root/*
sudo chmod 1775 ../BOINC_Installer/GR_Pkg_Root/Library

sudo chown -R 501:admin ../BOINC_Installer/GR_Pkg_Root/Library/Application\ Support/*
sudo chmod -R 755 ../BOINC_Installer/GR_Pkg_Root/Library/Application\ Support/*

sudo chown -R root:admin ../BOINC_Installer/GR_Installer\ Resources/*
sudo chmod -R 755 ../BOINC_Installer/GR_Installer\ Resources/*

sudo rm -dfR ../BOINC_Installer/New_Release_GR_$1_$2_$3/

mkdir -p ../BOINC_Installer/New_Release_GR_$1_$2_$3/
mkdir -p ../BOINC_Installer/New_Release_GR_$1_$2_$3/gridrepublic_$1.$2.$3_macOSX_powerpc
mkdir -p ../BOINC_Installer/New_Release_GR_$1_$2_$3/gridrepublic_$1.$2.$3_powerpc-apple-darwin
mkdir -p ../BOINC_Installer/New_Release_GR_$1_$2_$3/gridrepublic_$1.$2.$3_macOSX_SymbolTables

cp -fp mac_installer/GR-ReadMe.rtf ../BOINC_Installer/New_Release_GR_$1_$2_$3/gridrepublic_$1.$2.$3_macOSX_powerpc/ReadMe.rtf
sudo chown -R 501:admin ../BOINC_Installer/New_Release_GR_$1_$2_$3/gridrepublic_$1.$2.$3_macOSX_powerpc/ReadMe.rtf
sudo chmod -R 755 ../BOINC_Installer/New_Release_GR_$1_$2_$3/gridrepublic_$1.$2.$3_macOSX_powerpc/ReadMe.rtf

cp -fpR $BUILDPATH/boinc ../BOINC_Installer/New_Release_GR_$1_$2_$3/gridrepublic_$1.$2.$3_powerpc-apple-darwin/
cp -fpR $BUILDPATH/boinc_cmd ../BOINC_Installer/New_Release_GR_$1_$2_$3/gridrepublic_$1.$2.$3_powerpc-apple-darwin/
sudo chown -R root:admin ../BOINC_Installer/New_Release_GR_$1_$2_$3/gridrepublic_$1.$2.$3_powerpc-apple-darwin/*
sudo chmod -R 755 ../BOINC_Installer/New_Release_GR_$1_$2_$3/gridrepublic_$1.$2.$3_powerpc-apple-darwin/*

cp -fpR $BUILDPATH/SymbolTables ../BOINC_Installer/New_Release_GR_$1_$2_$3/gridrepublic_$1.$2.$3_macOSX_SymbolTables/

/Developer/Tools/packagemaker -build -p ../BOINC_Installer/New_Release_GR_$1_$2_$3/gridrepublic_$1.$2.$3_macOSX_powerpc/GridRepublic.pkg -f ../BOINC_Installer/GR_Pkg_Root -r ../BOINC_Installer/GR_Installer\ Resources/ -i mac_build/GR-Pkg-Info.plist -d mac_Installer/GR-Description.plist -ds 

cd ../BOINC_Installer/New_Release_GR_$1_$2_$3
zip -rq gridrepublic_$1.$2.$3_macOSX_powerpc.zip gridrepublic_$1.$2.$3_macOSX_powerpc
zip -rq gridrepublic_$1.$2.$3_powerpc-apple-darwin.zip gridrepublic_$1.$2.$3_powerpc-apple-darwin
zip -rq gridrepublic_$1.$2.$3_macOSX_SymbolTables.zip gridrepublic_$1.$2.$3_macOSX_SymbolTables

popd
return 0
