#!/bin/csh

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

##
# Release Script for Macintosh GridRepublic Desktop by Charlie Fenton
## updated 5/20/09 by Charlie Fenton
##

## NOTE: This script is deprecated.
## It does not put the branded skin into the installer package.
##
## Use the make_GridRepublic.sh script instead!

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

## For different branding, modify the following 9 variables:
PR_PATH="../BOINC_Installer/GR_Pkg_Root"
IR_PATH="../BOINC_Installer/GR_Installer_Resources"
SCRIPTS_PATH="../BOINC_Installer/GR_Installer\ Scripts"
NEW_DIR_PATH="../BOINC_Installer/New_Release_GR_$1_$2_$3"
README_FILE="mac_installer/GR-ReadMe.rtf"
BRANDING_FILE="mac_installer/GR-Branding"
ICNS_FILE="gridrepublic.icns"
INSTALLER_ICNS_FILE="GR_install.icns"
UNINSTALLER_ICNS_FILE="GR_uninstall.icns"
SAVER_SYSPREF_ICON_PATH="clientgui/mac/gridrepublic.tiff"
SAVER_LOGO_PATH="clientscr/res/gridrepublic_ss_logo.png"
BRAND_NAME="GridRepublic"
MANAGER_NAME="GridRepublic Desktop"
LC_BRAND_NAME="gridrepublic"

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

sudo rm -dfR "${IR_PATH}"
sudo rm -dfR "${PR_PATH}"
sudo rm -dfR "${SCRIPTS_PATH}"

mkdir -p "${IR_PATH}"
mkdir -p "${SCRIPTS_PATH}"

cp -fp mac_Installer/License.rtf "${IR_PATH}/"
cp -fp "${README_FILE}" "${IR_PATH}/ReadMe.rtf"
cp -fp win_build/installerv2/redist/all_projects_list.xml "${IR_PATH}/"

# Update version number
sed -i "" s/"<VER_NUM>"/"$1.$2.$3"/g "${IR_PATH}/ReadMe.rtf"

# Create the installer's preinstall and preupgrade scripts from the standard preinstall script
cp -fp mac_installer/preinstall "${SCRIPTS_PATH}/"

sed -i "" s/BOINCManager/"${MANAGER_NAME}"/g "${SCRIPTS_PATH}/preinstall"
sed -i "" s/BOINCSaver/"${BRAND_NAME}"/g "${SCRIPTS_PATH}/preinstall"

##### We've decided not to customize BOINC Data directory name for branding
#### sed -i "" s/BOINC/temp/g "${SCRIPTS_PATH}/preinstall"
#### sed -i "" s/"${BRAND_NAME}"/BOINC/g "${SCRIPTS_PATH}/preinstall"
#### sed -i "" s/temp/"${BRAND_NAME}"/g "${SCRIPTS_PATH}/preinstall"

cp -fp "${SCRIPTS_PATH}/preinstall" "${SCRIPTS_PATH}/preupgrade"

cp -fp mac_installer/postinstall "${SCRIPTS_PATH}/"
cp -fp mac_installer/postupgrade "${SCRIPTS_PATH}/"

cp -fpR "$BUILDPATH/PostInstall.app" "${IR_PATH}/"

mkdir -p "${PR_PATH}"
mkdir -p "${PR_PATH}/Applications"
mkdir -p "${PR_PATH}/Library"
mkdir -p "${PR_PATH}/Library/Screen Savers"
mkdir -p "${PR_PATH}/Library/Application Support"

##### We've decided not to customize BOINC Data directory name for branding
#### mkdir -p "${PR_PATH}/Library/Application Support/${BRAND_NAME} Data"
#### mkdir -p "${PR_PATH}/Library/Application Support/${BRAND_NAME} Data/locale"
mkdir -p "${PR_PATH}/Library/Application Support/BOINC Data"
mkdir -p "${PR_PATH}/Library/Application Support/BOINC Data/locale"
mkdir -p "${PR_PATH}/Library/Application Support/BOINC Data/switcher"
mkdir -p "${PR_PATH}/Library/Application Support/BOINC Data/skins"

cp -fpR "$BUILDPATH/switcher" "${PR_PATH}/Library/Application Support/BOINC Data/switcher/"
cp -fpR "$BUILDPATH/setprojectgrp" "${PR_PATH}/Library/Application Support/BOINC Data/switcher/"
## cp -fpR "$BUILDPATH/AppStats" "${PR_PATH}/Library/Application Support/BOINC Data/switcher/"

## Put Branding file into BOINC Data folder to make it available to screensaver 
cp -fp "${BRANDING_FILE}" "${PR_PATH}/Library/Application Support/BOINC Data/Branding"
cp -fp curl/ca-bundle.crt  "${PR_PATH}/Library/Application Support/BOINC Data/"

cp -fp doc/logo/boinc_logo_black.jpg "${PR_PATH}/Library/Application Support/BOINC Data/"
cp -fp api/txf/Helvetica.txf "${PR_PATH}/Library/Application Support/BOINC Data/"
cp -fp clientscr/ss_config.xml "${PR_PATH}/Library/Application Support/BOINC Data/"
cp -fpR "$BUILDPATH/boincscr" "${PR_PATH}/Library/Application Support/BOINC Data/"

cp -fpR "$BUILDPATH/BOINCManager.app" "${PR_PATH}/Applications/"

cp -fpR "$BUILDPATH/BOINCSaver.saver" "${PR_PATH}/Library/Screen Savers/"

## Copy the localization files into the installer tree

## Old way copies CVS and *.po files which are not needed
## cp -fpR locale/ "${PR_PATH}/Library/Application Support/BOINC Data/locale"
## sudo rm -dfR "${PR_PATH}/Library/Application Support/BOINC Data/locale/CVS"

## New way copies only *.mo files (adapted from boinc/sea/make-tar.sh)
##### We've decided not to customize BOINC Data directory name for branding
#### find locale/client -name '*.mo' | cut -d '/' -f 3 | awk -v PRPATH=${PR_PATH} -v BRANDNAME=${BRAND_NAME} '{print "\"" PRPATH "/Library/Application Support/" BRANDNAME " Data/locale/"$0"\""}' | xargs mkdir -p 
#### find locale/client -name '*.mo' | cut -d '/' -f 3,4 | awk -v PRPATH=${PR_PATH} -v BRANDNAME=${BRAND_NAME} '{print "cp \"locale/"$0"\" \"" PRPATH "/Library/Application Support/" BRANDNAME " Data/locale/"$0"\""}' | bash
find locale/client -name '*.mo' | cut -d '/' -f 2 | awk -v PRPATH=${PR_PATH} '{print "\"" PRPATH "/Library/Application Support/BOINC Data/locale/"$0"\""}' | xargs mkdir -p 
find locale/client -name '*.mo' | cut -d '/' -f 2,3 | awk -v PRPATH=${PR_PATH} '{print "cp \"locale/"$0"\" \"" PRPATH "/Library/Application Support/BOINC Data/locale/"$0"\""}' | bash

## Modify for Grid Republic
# Rename the Manager's bundle and its executable inside the bundle
mv -f "${PR_PATH}/Applications/BOINCManager.app/" "${PR_PATH}/Applications/${MANAGER_NAME}.app/"
mv -f "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/MacOS/BOINCManager" "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/MacOS/${MANAGER_NAME}"

# Update the Manager's info.plist, InfoPlist.strings files
sed -i "" s/BOINCManager/"${MANAGER_NAME}"/g "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Info.plist"
sed -i "" s/BOINCMgr.icns/"${ICNS_FILE}"/g "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Info.plist"
sed -i "" s/BOINC/"${BRAND_NAME}"/g "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Resources/English.lproj/InfoPlist.strings"

# Replace the Manager's BOINCMgr.icns file
cp -fp "clientgui/res/${ICNS_FILE}" "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Resources/${ICNS_FILE}"
rm -f "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Resources/BOINCMgr.icns"

# Copy Branding file into both Application Bundle and Installer Package\
cp -fp "${BRANDING_FILE}" "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Resources/Branding"
cp -fp "${BRANDING_FILE}" "${IR_PATH}/Branding"

# Rename the screensaver bundle and its executable inside the bundle
mv -f "${PR_PATH}/Library/Screen Savers/BOINCSaver.saver" "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver"
mv -f "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/MacOS/BOINCSaver" "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/MacOS/${BRAND_NAME}"

# Update screensaver's info.plist, InfoPlist.strings files
sed -i "" s/BOINCSaver/"${BRAND_NAME}"/g "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Info.plist"
sed -i "" s/BOINC/"${BRAND_NAME}"/g "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Resources/English.lproj/InfoPlist.strings"

# Replace screensaver's boinc.tiff or boinc.jpg file
rm -f "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Resources/boinc.jpg"
cp -fp "${SAVER_SYSPREF_ICON_PATH}" "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Resources/boinc.tiff"

# Replace screensaver's boinc_ss_logo.png file
rm -f "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Resources/boinc_ss_logo.png"
cp -fp "${SAVER_LOGO_PATH}" "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Resources/boinc_ss_logo.png"

## Fix up ownership and permissions
sudo chown -R root:admin "${PR_PATH}"/*
sudo chmod -R u+rw,g+rw,o+r-w "${PR_PATH}"/*
sudo chmod 1775 "${PR_PATH}/Library"

sudo chown -R 501:admin "${PR_PATH}/Library/Application Support"/*
sudo chmod -R u+rw,g+r-w,o+r-w "${PR_PATH}/Library/Application Support"/*

sudo chown -R root:admin "${IR_PATH}"/*
sudo chown -R root:admin "${SCRIPTS_PATH}"/*
sudo chmod -R u+rw,g+r-w,o+r-w "${IR_PATH}"/*
sudo chmod -R u+rw,g+r-w,o+r-w "${SCRIPTS_PATH}"/*

sudo rm -dfR "${NEW_DIR_PATH}/"

mkdir -p "${NEW_DIR_PATH}/"
mkdir -p "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal"
mkdir -p "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras"

cp -fp "${IR_PATH}/ReadMe.rtf" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/ReadMe.rtf"
sudo chown -R 501:admin "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/ReadMe.rtf"
sudo chmod -R 644 "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/ReadMe.rtf"

cp -fp "COPYING" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras"
sudo chown -R 501:admin "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/COPYING"
sudo chmod -R 644 "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/COPYING"

cp -fp "COPYING.LESSER" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras"
sudo chown -R 501:admin "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/COPYING.LESSER"
sudo chmod -R 644 "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/COPYING.LESSER"

cp -fp "COPYRIGHT" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/"
sudo chown -R 501:admin "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/COPYRIGHT"
sudo chmod -R 644 "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/COPYRIGHT"

# Copy & rename the Uninstall application's bundle and rename its executable inside the bundle
sudo cp -fpR "$BUILDPATH/Uninstall BOINC.app" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app"
sudo mv -f "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app/Contents/MacOS/Uninstall BOINC" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app/Contents/MacOS/Uninstall ${BRAND_NAME}"

# Update Uninstall application's info.plist, InfoPlist.strings files
sudo sed -i "" s/BOINC/"${BRAND_NAME}"/g "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app/Contents/Info.plist"
sudo sed -i "" s/MacUninstaller.icns/"${UNINSTALLER_ICNS_FILE}"/g "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app/Contents/Info.plist"
#### sed -i "" s/BOINC/"${BRAND_NAME}"/g "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app/Contents/Resources/English.lproj/InfoPlist.strings"

# Replace the Uninstall application's MacUninstaller.icns file
sudo cp -fp "clientgui/res/${UNINSTALLER_ICNS_FILE}" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app/Contents/Resources/${UNINSTALLER_ICNS_FILE}"
sudo rm -f "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app/Contents/Resources/MacUninstaller.icns"
# Remove the Uninstall application's resource file so it will show generic "Are you sure?" dialog
sudo rm -f "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app/Contents/Resources/Uninstall BOINC.rsrc"

sudo chown -R root:admin "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app"
sudo chmod -R 555 "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app"

##### We've decided not to create branded command-line executables; they are identical to standard ones
#### mkdir -p "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_universal-apple-darwin"
#### cp -fpR $BUILDPATH/boinc "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_universal-apple-darwin/"
#### cp -fpR $BUILDPATH/boinccmd "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_universal-apple-darwin/"
#### cp -fpR curl/ca-bundle.crt "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_universal-apple-darwin/"
#### sudo chown -R root:admin "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_universal-apple-darwin"/*
#### sudo chmod -R u+rw-s,g+r-ws,o+r-w "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_universal-apple-darwin"/*

##### We've decided not to create branded symbol table file; it is identical to standard one
#### mkdir -p "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_SymbolTables"
#### cp -fpR $BUILDPATH/SymbolTables/ ${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_SymbolTables/

# Make temporary copies of Pkg-Info.plist and Description.plist for PackageMaker and update for this branding
cp -fp mac_build/Pkg-Info.plist "${NEW_DIR_PATH}"
cp -fp mac_Installer/Description.plist "${NEW_DIR_PATH}"
sed -i "" s/BOINC/"${BRAND_NAME}"/g "${NEW_DIR_PATH}/Pkg-Info.plist"
sed -i "" s/BOINC/"${BRAND_NAME}"/g "${NEW_DIR_PATH}/Description.plist"

# Copy the installer wrapper application "${BRAND_NAME} Installer.app"
cp -fpR "$BUILDPATH/BOINC Installer.app" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app"

# Update the installer wrapper application's info.plist, InfoPlist.strings files
sed -i "" s/BOINC/"${BRAND_NAME}"/g "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Info.plist"
sed -i "" s/MacInstaller.icns/"${INSTALLER_ICNS_FILE}"/g "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Info.plist"
## sed -i "" s/BOINC/"${BRAND_NAME}"/g "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Resources/English.lproj/InfoPlist.strings"

# Replace the installer wrapper application's MacInstaller.icns file
cp -fp "clientgui/res/${INSTALLER_ICNS_FILE}" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Resources/${INSTALLER_ICNS_FILE}"
rm -f "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Resources/MacInstaller.icns"

# Rename the installer wrapper application's executable inside the bundle
mv -f "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/MacOS/BOINC Installer" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/MacOS/${BRAND_NAME} Installer"

DarwinVersion=`uname -r`;
DarwinMajorVersion=`echo $DarwinVersion | sed 's/\([0-9]*\)[.].*/\1/' `;
# DarwinMinorVersion=`echo $version | sed 's/[0-9]*[.]\([0-9]*\).*/\1/' `;
#
# echo "major = $DarwinMajorVersion"
# echo "minor = $DarwinMinorVersion"
#
# Darwin version 9.x.y corresponds to OS 10.5.x
# Darwin version 8.x.y corresponds to OS 10.4.x
# Darwin version 7.x.y corresponds to OS 10.3.x
# Darwin version 6.x corresponds to OS 10.2.x

# Build the installer package inside the wrapper application's bundle
if [ "$DarwinMajorVersion" = "9" ]; then
    # OS 10.5 packagemaker
    /Developer/usr/bin/packagemaker -r "${PR_PATH}" -e "${IR_PATH}" -s "${SCRIPTS_PATH}" -f "${NEW_DIR_PATH}/Pkg-Info.plist" -t "${MANAGER_NAME}" -n "$1.$2.$3" -b -o "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg"
    # Remove TokenDefinitions.plist which, along with IFPkgPathMappings in Info.plist, would cause installer to find a previous copy of BOINCManager and install there
    sudo rm -f "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg/Contents/Resources/TokenDefinitions.plist"
    defaults delete "$PWD/${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg/Contents/Info" IFPkgPathMappings
else
    # OS 10.4 packagemaker
    /Developer/Tools/packagemaker -build -p "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg" -f "${PR_PATH}" -r "${IR_PATH}" -i "${NEW_DIR_PATH}/Pkg-Info.plist" -d "${NEW_DIR_PATH}/Description.plist" -ds 
fi

# Allow the installer wrapper application to modify the package's Info.plist file
sudo chmod u+w,g+w,o+w "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg/Contents/Info.plist"

# Remove temporary copies of Pkg-Info.plist and Description.plist
rm ${NEW_DIR_PATH}/Pkg-Info.plist
rm ${NEW_DIR_PATH}/Description.plist

# Compress the products
cd ${NEW_DIR_PATH}
zip -rqy ${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal.zip ${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal
##### We've decided not to create branded command-line executables; they are identical to standard ones
#### zip -rqy ${LC_BRAND_NAME}_$1.$2.$3_universal-apple-darwin.zip ${LC_BRAND_NAME}_$1.$2.$3_universal-apple-darwin
##### We've decided not to create branded symbol table file; it is identical to standard one
#### zip -rqy ${LC_BRAND_NAME}_$1.$2.$3_macOSX_SymbolTables.zip ${LC_BRAND_NAME}_$1.$2.$3_macOSX_SymbolTables

# Force Finder to recognize changed icons by deleting the uncompressed products and expanding the zip file 
sudo rm -dfR ${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal
open ${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal.zip

popd
return 0
