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
# Script to convert Macintosh BOINC installer to GridRepublic Desktop installer
# updated 12/7/11 by Charlie Fenton for BOINC 6.8.33 / 6.12.44 / 7.0.3 and later
# updated 8/2/12 by Charlie Fenton to code sign the installer and uninstaller
##

## Usage:
## First put the following files into a working directory:
##     the BOINC installer to be converted
##     the Uninstall BOINC application to be converted
##     GR_ReadMe.rtf
##     gridrepublic.icns
##     GR_install.icns
##     GR_uninstall.icns
##     COPYING
##     COPYING.LESSER
##     COPYRIGHT
##     skins directory containing GridRepublic skin (optional)
##     acct_mgr_url.xml (to have BOINC automatically connect to Account Manager)
##     gridrepublic.tiff (for screensaver coordinator)
##     gridrepublic_ss_logo.png (for screensaver coordinator)
##     GR_saver directory containing GridRepublic default screensaver and associated files, including:
##          gridrepublic_ss_logo.jpg
##
## NOTE: This script requires Mac OS 10.6 or later, and uses XCode developer
##   tools.  So you must have installed XCode Developer Tools on the Mac 
##   before running this script.
##
## If you wish to code sign the installer and uninstaller, create a file 
## ~/BOINCCodeSignIdentity.txt whose first line is the code signing identity
##
## cd to the working directory:
##
## Invoke this script with the three parts of version number as arguments.  
## For example, if the version is 3.2.1:
##     sh [path_to_this_script] 3 2 1
##
## This will create a directory "BOINC_Installer" in the parent directory of 
## the current directory
##
## For different branding, modify the following 9 variables:
PR_PATH="GR_Pkg_Root"
IR_PATH="GR_Installer_Resources"
SCRIPTS_PATH="GR_Installer_Scripts"
NEW_DIR_PATH="New_Release_GR_$1_$2_$3"
README_FILE="GR-ReadMe.rtf"
## BRANDING_FILE="GR-Branding"
BRANDING_INFO="BrandId=1"
ICNS_FILE="gridrepublic.icns"
INSTALLER_ICNS_FILE="GR_install.icns"
UNINSTALLER_ICNS_FILE="GR_uninstall.icns"
SAVER_DIR="GR_saver"
SAVER_SYSPREF_ICON="gridrepublic.tiff"
SAVER_LOGO="gridrepublic_ss_logo.png"
BRAND_NAME="GridRepublic"
MANAGER_NAME="GridRepublic Desktop"
LC_BRAND_NAME="gridrepublic"
SOURCE_PKG_PATH="BOINC Installer.app/Contents/Resources/BOINC.pkg/Contents"

if [ $# -lt 3 ]; then
echo "Usage:"
echo "   cd working_directory"
echo "   sh [path_to_this_script] major_version minor_version revision_number"
echo ""
echo "See comments at start of script for more info."
echo ""
exit 1
fi

#pushd ./
WorkingDirPath=$PWD

## Make sure sed uses UTF-8 text encoding
unset LC_CTYPE
unset LC_MESSAGES
unset __CF_USER_TEXT_ENCODING
export LANG=en_US.UTF-8

sudo rm -dfR "${IR_PATH}"
sudo rm -dfR "${PR_PATH}"
sudo rm -dfR "${SCRIPTS_PATH}"

mkdir -p "${IR_PATH}"
mkdir -p "${PR_PATH}"
mkdir -p "${SCRIPTS_PATH}"

sudo rm -dfR "${NEW_DIR_PATH}/"

mkdir -p "${NEW_DIR_PATH}/"
mkdir -p "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686"
mkdir -p "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras"

cp -fp "${SOURCE_PKG_PATH}/Archive.pax.gz" "${PR_PATH}/"
cd "${PR_PATH}"
sudo gunzip ./Archive.pax.gz
sudo pax -r -pe -f Archive.pax
rm -df "Archive.pax"
cd ..

cp -fp "${SOURCE_PKG_PATH}/Resources/License.rtf" "${IR_PATH}/"
cp -fp "${README_FILE}" "${IR_PATH}/ReadMe.rtf"
# Update version number
sed -i "" s/"<VER_NUM>"/"$1.$2.$3"/g "${IR_PATH}/ReadMe.rtf"

# Create the installer's preinstall and preupgrade scripts from the standard preinstall script
cp -fp "${SOURCE_PKG_PATH}/Resources/preinstall" "${SCRIPTS_PATH}/"

sed -i "" s/BOINCManager/"${MANAGER_NAME}"/g "${SCRIPTS_PATH}/preinstall"
sed -i "" s/BOINCSaver/"${BRAND_NAME}"/g "${SCRIPTS_PATH}/preinstall"

cp -fp "${SCRIPTS_PATH}/preinstall" "${SCRIPTS_PATH}/preupgrade"

cp -fp "${SOURCE_PKG_PATH}/Resources/postinstall" "${SCRIPTS_PATH}/"
cp -fp "${SOURCE_PKG_PATH}/Resources/postupgrade" "${SCRIPTS_PATH}/"

cp -fpR "${SOURCE_PKG_PATH}/Resources/PostInstall.app" "${IR_PATH}/"

cp -fp "${SOURCE_PKG_PATH}/Resources/all_projects_list.xml" "${IR_PATH}/"

##### We've decided not to customize BOINC Data directory name for branding
#### mkdir -p "${PR_PATH}/Library/Application Support/${BRAND_NAME} Data"
#### mkdir -p "${PR_PATH}/Library/Application Support/${BRAND_NAME} Data/locale"

mkdir -p "${PR_PATH}/Library/Application Support/BOINC Data"

## If skins folder is present. copy it into BOINC Data folder
if [ -d "skins" ]; then
    sudo cp -fR "skins" "${PR_PATH}/Library/Application Support/BOINC Data/"
fi

## Normally, we would put the account manager URL file into the Package Root folder 
## for delivery to the BOINC Data folder.  But if the user later installs standard 
## BOINC (without this file), the Apple installer would then delete the file.
## So we "hide" it in the installer's resources, and have the PostInstall script copy 
## it into the BOINC Data folder
##
## If account manager URL file is present, copy it into installer resources for 
## eventual delivery into the BOINC Data folder
if [ -f "acct_mgr_url.xml" ]; then
##    sudo cp -fR "acct_mgr_url.xml" "${PR_PATH}/Library/Application Support/BOINC Data/acct_mgr_url.xml"
    sudo cp -fR "acct_mgr_url.xml" "${IR_PATH}/"
fi

## Modify for Grid Republic
# Rename the Manager's bundle and its executable inside the bundle
sudo mv -f "${PR_PATH}/Applications/BOINCManager.app/" "${PR_PATH}/Applications/${MANAGER_NAME}.app/"
sudo mv -f "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/MacOS/BOINCManager" "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/MacOS/${MANAGER_NAME}"

# Update the Manager's info.plist, InfoPlist.strings files
sudo sed -i "" s/BOINCManager/"${MANAGER_NAME}"/g "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Info.plist"
sudo sed -i "" s/BOINCMgr.icns/"${ICNS_FILE}"/g "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Info.plist"

sudo chmod a+w "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Resources/English.lproj/InfoPlist.strings"
sudo sed -i "" s/BOINC/"${MANAGER_NAME}"/g "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Resources/English.lproj/InfoPlist.strings"

# Replace the Manager's BOINCMgr.icns file
sudo cp -fp "${ICNS_FILE}" "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Resources/${ICNS_FILE}"
sudo rm -f "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Resources/BOINCMgr.icns"

# Put Branding file in both Installer Package and Application Bundle
sudo echo ${BRANDING_INFO} > "${IR_PATH}/Branding"
sudo cp -fp "${IR_PATH}/Branding" "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Resources/Branding"

## Put Branding file into BOINC Data folder to make it available to screensaver coordinator
sudo cp -fp "${IR_PATH}/Branding" "${PR_PATH}/Library/Application Support/BOINC Data/Branding"

# Rename the screensaver coordinator bundle and its executable inside the bundle
sudo mv -f "${PR_PATH}/Library/Screen Savers/BOINCSaver.saver" "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver"
sudo mv -f "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/MacOS/BOINCSaver" "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/MacOS/${BRAND_NAME}"

# Update screensaver coordinator's info.plist, InfoPlist.strings files
sudo sed -i "" s/BOINCSaver/"${BRAND_NAME}"/g "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Info.plist"

sudo chmod a+w "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Resources/English.lproj/InfoPlist.strings"
sudo sed -i "" s/BOINC/"${BRAND_NAME}"/g "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Resources/English.lproj/InfoPlist.strings"

# Replace screensaver coordinator's boinc.tiff or boinc.jpg file
sudo rm -f "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Resources/boinc.jpg"
sudo cp -fp "${SAVER_SYSPREF_ICON}" "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Resources/boinc.tiff"

# Replace screensaver coordinator's boinc_ss_logo.png file
sudo rm -f "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Resources/boinc_ss_logo.png"
sudo cp -fp "${SAVER_LOGO}" "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Resources/boinc_ss_logo.png"

# Delete the BOINC default screensaver and its associated files
sudo rm -f "${PR_PATH}/Library/Application Support/BOINC Data/boinc_logo_black.jpg"

# Copy the GridRepublic default screensaver files into BOINC Data folder
sudo cp -fR "${SAVER_DIR}/" "${PR_PATH}/Library/Application Support/BOINC Data/"

# Copy and rename the Uninstall application's bundle and rename its executable inside the bundle
sudo cp -fpR "Uninstall BOINC.app" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/Uninstall ${BRAND_NAME}.app"
sudo mv -f "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/Uninstall ${BRAND_NAME}.app/Contents/MacOS/Uninstall BOINC" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/Uninstall ${BRAND_NAME}.app/Contents/MacOS/Uninstall ${BRAND_NAME}"

# Update Uninstall application's info.plist, InfoPlist.strings files
sudo sed -i "" s/BOINC/"${BRAND_NAME}"/g "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/Uninstall ${BRAND_NAME}.app/Contents/Info.plist"
sudo sed -i "" s/MacUninstaller.icns/"${UNINSTALLER_ICNS_FILE}"/g "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/Uninstall ${BRAND_NAME}.app/Contents/Info.plist"

sudo chmod a+w "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/Uninstall ${BRAND_NAME}.app/Contents/Resources/English.lproj/InfoPlist.strings"
sudo sed -i "" s/BOINC/"${BRAND_NAME}"/g "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/Uninstall ${BRAND_NAME}.app/Contents/Resources/English.lproj/InfoPlist.strings"

# Replace the Uninstall application's MacUninstaller.icns file
sudo cp -fp "${UNINSTALLER_ICNS_FILE}" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/Uninstall ${BRAND_NAME}.app/Contents/Resources/${UNINSTALLER_ICNS_FILE}"
sudo rm -f "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/Uninstall ${BRAND_NAME}.app/Contents/Resources/MacUninstaller.icns"
# Remove the Uninstall application's resource file so it will show generic "Are you sure?" dialog
sudo rm -f "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/Uninstall ${BRAND_NAME}.app/Contents/Resources/Uninstall BOINC.rsrc"

sudo chown -R root:admin "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/Uninstall ${BRAND_NAME}.app"
sudo chmod -R 755 "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/Uninstall ${BRAND_NAME}.app"

## Fix up ownership and permissions
sudo chown -R root:admin "${PR_PATH}"/*
sudo chmod -R u+rw,g+rw,o+r-w "${PR_PATH}"/*
sudo chmod 1775 "${PR_PATH}/Library"

sudo chown -R 501:admin "${PR_PATH}/Library/Application Support"/*
sudo chmod -R u+rw,g+r-w,o+r-w "${PR_PATH}/Library/Application Support"/*

sudo chown -R root:admin "${IR_PATH}"/*
sudo chmod -R u+rw,g+r-w,o+r-w "${IR_PATH}"/*
sudo chown -R root:admin "${SCRIPTS_PATH}"/*
sudo chmod -R u+rw,g+r-w,o+r-w "${SCRIPTS_PATH}"/*

sudo cp -fp "${IR_PATH}/ReadMe.rtf" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/ReadMe.rtf"
sudo chown -R 501:admin "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/ReadMe.rtf"
sudo chmod -R 644 "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/ReadMe.rtf"
sudo cp -fp "COPYING" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/COPYING.txt"
sudo chown -R 501:admin "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/COPYING.txt"
sudo chmod -R 644 "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/COPYING.txt"
sudo cp -fp "COPYRIGHT" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/COPYRIGHT.txt"
sudo chown -R 501:admin "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/COPYRIGHT.txt"
sudo chmod -R 644 "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/COPYRIGHT.txt"

# COPYING.LESSER is part of GNU License v3
sudo cp -fp "COPYING.LESSER" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/COPYING.LESSER.txt"
sudo chown -R 501:admin "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/COPYING.LESSER.txt"
sudo chmod -R 644 "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/COPYING.LESSER.txt"

# Make temporary copies of Pkg-Info.plist and Description.plist for PackageMaker and update for this branding
sudo cp -fp "${SOURCE_PKG_PATH}/Info.plist" "${NEW_DIR_PATH}/Pkg-Info.plist"
sudo chown -R 501:admin "${NEW_DIR_PATH}/Pkg-Info.plist"
sudo chmod -R 666 "${NEW_DIR_PATH}/Pkg-Info.plist"
if [ -f "${SOURCE_PKG_PATH}/Resources/English.lproj/Description.plist" ]; then
    sudo cp -fp "${SOURCE_PKG_PATH}/Resources/English.lproj/Description.plist" "${NEW_DIR_PATH}"
else
    sudo cp -fp "${SOURCE_PKG_PATH}/Resources/en.lproj/Description.plist" "${NEW_DIR_PATH}"
fi
sudo chown -R 501:admin "${NEW_DIR_PATH}/Description.plist"
sudo chmod -R 666 "${NEW_DIR_PATH}/Description.plist"

# Update Pkg-Info.plist name and ensure it is in XML format
defaults write "`pwd`/${NEW_DIR_PATH}/Pkg-Info" "CFBundleGetInfoString" "$BRAND_NAME $1.$2.$3"
plutil -convert xml1 "`pwd`/${NEW_DIR_PATH}/Pkg-Info.plist"

# Update Description.plist name and ensure it is in XML format
defaults write "`pwd`/${NEW_DIR_PATH}/Description" "IFPkgDescriptionTitle" "$MANAGER_NAME"
plutil -convert xml1 "`pwd`/${NEW_DIR_PATH}/Description.plist"

# Copy the installer wrapper application "${BRAND_NAME} Installer.app"
sudo cp -fpR "BOINC Installer.app" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app"
sudo rm -dfR "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Resources/BOINC.pkg"

# Update the installer wrapper application's info.plist, InfoPlist.strings files
sudo sed -i "" s/BOINC/"${BRAND_NAME}"/g "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Info.plist"
sudo sed -i "" s/MacInstaller.icns/"${INSTALLER_ICNS_FILE}"/g "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Info.plist"

sudo chmod a+w "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Resources/English.lproj/InfoPlist.strings"
sudo sed -i "" s/BOINC/"${MANAGER_NAME}"/g "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Resources/English.lproj/InfoPlist.strings"

# Replace the installer wrapper application's MacInstaller.icns file
sudo cp -fp "${INSTALLER_ICNS_FILE}" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Resources/${INSTALLER_ICNS_FILE}"
sudo rm -f "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Resources/MacInstaller.icns"

# Rename the installer wrapper application's executable inside the bundle
sudo mv -f "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/MacOS/BOINC Installer" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/MacOS/${BRAND_NAME} Installer"

# Build the installer package inside the wrapper application's bundle
# Because PackageMaker is now distributed separately from Xcode, we 
# emulate the following PackageMaker command:
###/Developer/usr/bin/packagemaker -r "${PR_PATH}" -e "${IR_PATH}" -s "${SCRIPTS_PATH}" -f "${NEW_DIR_PATH}/Pkg-Info.plist" -t "${MANAGER_NAME}" -n "$1.$2.$3" -b -o "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg"
# Our PackageMaker emulation starts here
mkdir -p "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg/Contents/Resources"

cd "${PR_PATH}"

mkbom ./ "${WorkingDirPath}/${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg/Contents/Archive.bom"

pax -wz -x cpio -f "${WorkingDirPath}/${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg/Contents/Archive.pax.gz" ./

cd "${WorkingDirPath}"

echo "pmkrpkg1" >> "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg/Contents/PkgInfo"

cat >> "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg/Contents/Resources/package_version" << ENDOFFILE
major: $1
minor: $2
ENDOFFILE

cp -fp "${NEW_DIR_PATH}/Pkg-Info.plist" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg/Contents/Info.plist"

cp -fpR "${IR_PATH}/" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg/Contents/Resources"

sudo chmod a+x "${SCRIPTS_PATH}"/*
cp -fpR "${SCRIPTS_PATH}/" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg/Contents/Resources"

# End of our PackageMaker emulation

## for debugging
## if [  $? -ne 0 ]; then
## echo ""
## echo "********** /Pkg-Info.plist File contents: *************"
## echo ""
## cp "${NEW_DIR_PATH}/Pkg-Info.plist" /dev/stdout
## echo ""
## echo "********** End /Pkg-Info.plist File contents *************"
## echo ""
## fi

# Allow the installer wrapper application to modify the package's Info.plist file
sudo chmod u+w,g+w,o+w "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg/Contents/Info.plist"

# add a more complete Description.plist file to display in Installer's Customize pane
if [ ! -d "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg/Contents/Resources/en.lproj/" ]; then
    mkdir -p "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg/Contents/Resources/en.lproj"
fi
cp -fp "${NEW_DIR_PATH}/Description.plist" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg/Contents/Resources/en.lproj/"

# Update the installer wrapper application's creation date
sudo touch "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app"

# Remove temporary copies of Pkg-Info.plist and Description.plist
sudo rm ${NEW_DIR_PATH}/Pkg-Info.plist
sudo rm ${NEW_DIR_PATH}/Description.plist

# Remove temporary directories
sudo rm -dfR "${IR_PATH}"
sudo rm -dfR "${PR_PATH}"
sudo rm -dfR "${SCRIPTS_PATH}"

## If you wish to code sign the installer and uninstaller, create a file 
## ~/BOINCCodeSignIdentity.txt whose first line is the code signing identity
##
## Code signing using a registered Apple Developer ID is necessary for GateKeeper 
## with default settings to allow running downloaded applications under OS 10.8
if [ -e "${HOME}/BOINCCodeSignIdentity.txt" ]; then
    exec 8<"${HOME}/BOINCCodeSignIdentity.txt"
    read -u 8 SIGNINGIDENTITY

    # Code Sign the BOINC installer if we have a signing identity
    sudo codesign -f -s "${SIGNINGIDENTITY}" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/${BRAND_NAME} Installer.app"

    # Code Sign the BOINC uninstaller if we have a signing identity
    sudo codesign -f -s "${SIGNINGIDENTITY}" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686/extras/Uninstall ${BRAND_NAME}.app"
fi

# Compress the products
cd ${NEW_DIR_PATH}
## Use ditto instead of zip utility to preserve resource forks and Finder attributes (custom icon, hide extension) 
ditto -ck --sequesterRsrc --keepParent "${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686" "${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686.zip"
##### We've decided not to create branded command-line executables; they are identical to standard ones
#### ditto -ck --sequesterRsrc --keepParent "${LC_BRAND_NAME}_$1.$2.$3_i686-apple-darwin" "${LC_BRAND_NAME}_$1.$2.$3_i686-apple-darwin.zip"
##### We've decided not to create branded symbol table file; it is identical to standard one
#### ditto -ck --sequesterRsrc --keepParent "${LC_BRAND_NAME}_$1.$2.$3_macOSX_SymbolTables" "${LC_BRAND_NAME}_$1.$2.$3_macOSX_SymbolTables.zip"

# Force Finder to recognize changed icons by deleting the uncompressed products and expanding the zip file 
sudo rm -dfR "${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686"
open "${LC_BRAND_NAME}_$1.$2.$3_macOSX_i686.zip"

#popd
cd "${WorkingDirPath}"
exit 0
