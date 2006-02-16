#!/bin/csh

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

## For different branding, modify the following 9 variables:
PR_PATH="../BOINC_Installer/GR_Pkg_Root"
IR_PATH="../BOINC_Installer/GR_Installer_Resources"
NEW_DIR_PATH="../BOINC_Installer/New_Release_GR_$1_$2_$3"
README_FILE="mac_installer/GR-ReadMe.rtf"
BRANDING_FILE="mac_installer/GR-Branding"
ICNS_FILE="GridRepublic.icns"
SAVER_SYSPREF_ICON_PATH="clientgui/mac/GridRepublic.tiff"
BRAND_NAME="GridRepublic"
LC_BRAND_NAME="gridrepublic"


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

sudo rm -dfR ${IR_PATH}
sudo rm -dfR ${PR_PATH}

mkdir -p ${IR_PATH}

cp -fp mac_Installer/License.rtf ${IR_PATH}/
cp -fp "${README_FILE}" ${IR_PATH}/ReadMe.rtf

# Create the installer's preinstall and preupgrade scripts from the standard preinstall script
cp -fp mac_installer/preinstall ${IR_PATH}/
sed -i "" s/BOINC/temp/g ${IR_PATH}/preinstall
sed -i "" s/${BRAND_NAME}/BOINC/g ${IR_PATH}/preinstall
sed -i "" s/temp/${BRAND_NAME}/g ${IR_PATH}/preinstall
cp -fp ${IR_PATH}/preinstall ${IR_PATH}/preupgrade

cp -fp mac_installer/postinstall ${IR_PATH}/
cp -fp mac_installer/postupgrade ${IR_PATH}/

cp -fpR $BUILDPATH/Postinstall.app ${IR_PATH}/

mkdir -p ${PR_PATH}
mkdir -p ${PR_PATH}/Applications
mkdir -p ${PR_PATH}/Library
mkdir -p ${PR_PATH}/Library/Screen\ Savers
mkdir -p ${PR_PATH}/Library/Application\ Support
mkdir -p ${PR_PATH}/Library/Application\ Support/${BRAND_NAME}\ Data
mkdir -p ${PR_PATH}/Library/Application\ Support/${BRAND_NAME}\ Data/locale

cp -fpR $BUILDPATH/BOINCManager.app ${PR_PATH}/Applications/

cp -fpR $BUILDPATH/BOINCSaver.saver ${PR_PATH}/Library/Screen\ Savers/

## Copy the localization files into the installer tree
## Old way copies CVS and *.po files which are not needed
## cp -fpR locale/client/ ${PR_PATH}/Library/Application\ Support/BOINC\ Data/locale
## sudo rm -dfR ${PR_PATH}/Library/Application\ Support/BOINC\ Data/locale/CVS
## New way copies only *.mo files (adapted from boinc/sea/make-tar.sh)
find locale/client -name '*.mo' | cut -d '/' -f 3 | awk -v PRPATH=${PR_PATH} -v BRANDNAME=${BRAND_NAME} '{print "\"" PRPATH "/Library/Application Support/" BRANDNAME " Data/locale/"$0"\""}' | xargs mkdir -p 
find locale/client -name '*.mo' | cut -d '/' -f 3,4 | awk -v PRPATH=${PR_PATH} -v BRANDNAME=${BRAND_NAME} '{print "cp \"locale/client/"$0"\" \"" PRPATH "/Library/Application Support/" BRANDNAME " Data/locale/"$0"\""}' | bash

## Modify for Grid Republic
# Rename the Manager's bundle and its executable inside the bundle
mv -f ${PR_PATH}/Applications/BOINCManager.app/ ${PR_PATH}/Applications/${BRAND_NAME}\ Manager.app/
mv -f ${PR_PATH}/Applications/${BRAND_NAME}\ Manager.app/Contents/MacOS/BOINCManager ${PR_PATH}/Applications/${BRAND_NAME}\ Manager.app/Contents/MacOS/${BRAND_NAME}\ Manager

# Update the Manager's info.plist, InfoPlist.strings files
sed -i "" s/BOINCManager/${BRAND_NAME}\ Manager/g ${PR_PATH}/Applications/${BRAND_NAME}\ Manager.app/Contents/Info.plist
sed -i "" s/BOINCMgr.icns/"${ICNS_FILE}"/g ${PR_PATH}/Applications/${BRAND_NAME}\ Manager.app/Contents/Info.plist
sed -i "" s/BOINC/${BRAND_NAME}/g ${PR_PATH}/Applications/${BRAND_NAME}\ Manager.app/Contents/Resources/English.lproj/InfoPlist.strings

# Replace the Manager's BOINCMgr.icns file
cp -fp "client/mac/${ICNS_FILE}" "${PR_PATH}/Applications/${BRAND_NAME} Manager.app/Contents/Resources/${ICNS_FILE}"
rm -f ${PR_PATH}/Applications/${BRAND_NAME}\ Manager.app/Contents/Resources/BOINCMgr.icns

# Copy Branding file into both Application Bundle and Installer Package
cp -fp "${BRANDING_FILE}" ${PR_PATH}/Applications/${BRAND_NAME}\ Manager.app/Contents/Resources/Branding
cp -fp "${BRANDING_FILE}" ${IR_PATH}/Branding

# Rename the screensaver bundle and its executable inside the bundle
mv -f ${PR_PATH}/Library/Screen\ Savers/BOINCSaver.saver ${PR_PATH}/Library/Screen\ Savers/${BRAND_NAME}.saver
mv -f ${PR_PATH}/Library/Screen\ Savers/${BRAND_NAME}.saver/Contents/MacOS/BOINCSaver ${PR_PATH}/Library/Screen\ Savers/${BRAND_NAME}.saver/Contents/MacOS/${BRAND_NAME}

# Update screensaver's info.plist, InfoPlist.strings files
sed -i "" s/BOINCSaver/${BRAND_NAME}/g ${PR_PATH}/Library/Screen\ Savers/${BRAND_NAME}.saver/Contents/Info.plist
sed -i "" s/BOINC/${BRAND_NAME}/g ${PR_PATH}/Library/Screen\ Savers/${BRAND_NAME}.saver/Contents/Resources/English.lproj/InfoPlist.strings

# Replace screensaver's boinc.tif file
cp -fp "${SAVER_SYSPREF_ICON_PATH}" ${PR_PATH}/Library/Screen\ Savers/${BRAND_NAME}.saver/Contents/Resources/boinc.tiff

## Fix up ownership and permissions
sudo chown -R root:admin ${PR_PATH}/*
sudo chmod -R 775 ${PR_PATH}/*
sudo chmod 1775 ${PR_PATH}/Library

sudo chown -R 501:admin ${PR_PATH}/Library/Application\ Support/*
sudo chmod -R 755 ${PR_PATH}/Library/Application\ Support/*

sudo chown -R root:admin ${IR_PATH}/*
sudo chmod -R 755 ${IR_PATH}/*

sudo rm -dfR ${NEW_DIR_PATH}/

mkdir -p ${NEW_DIR_PATH}/
mkdir -p ${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal
mkdir -p ${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_universal-apple-darwin
mkdir -p ${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_SymbolTables

cp -fp "${README_FILE}" ${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/ReadMe.rtf
sudo chown -R 501:admin ${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/ReadMe.rtf
sudo chmod -R 755 ${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/ReadMe.rtf

cp -fpR $BUILDPATH/boinc ${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_universal-apple-darwin/
cp -fpR $BUILDPATH/boinc_cmd ${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_universal-apple-darwin/
sudo chown -R root:admin ${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_universal-apple-darwin/*
sudo chmod -R 755 ${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_universal-apple-darwin/*

cp -fpR $BUILDPATH/SymbolTables ${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_SymbolTables/

# Make temporary copies of Pkg-Info.plist and Description.plist for PackageMaker and update for this branding
cp -fp mac_build/Pkg-Info.plist ${NEW_DIR_PATH}
cp -fp mac_Installer/Description.plist ${NEW_DIR_PATH}
sed -i "" s/BOINC/${BRAND_NAME}/g ${NEW_DIR_PATH}/Pkg-Info.plist
sed -i "" s/BOINC/${BRAND_NAME}/g ${NEW_DIR_PATH}/Description.plist

# Build the installer package
/Developer/Tools/packagemaker -build -p ${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME}.pkg -f ${PR_PATH} -r ${IR_PATH} -i ${NEW_DIR_PATH}/Pkg-Info.plist -d ${NEW_DIR_PATH}/Description.plist -ds 

# Remove temporary copies of Pkg-Info.plist and Description.plist
rm ${NEW_DIR_PATH}/Pkg-Info.plist
rm ${NEW_DIR_PATH}/Description.plist

# Compress the products
cd ${NEW_DIR_PATH}
zip -rq ${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal.zip ${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal
zip -rq ${LC_BRAND_NAME}_$1.$2.$3_universal-apple-darwin.zip ${LC_BRAND_NAME}_$1.$2.$3_universal-apple-darwin
zip -rq ${LC_BRAND_NAME}_$1.$2.$3_macOSX_SymbolTables.zip ${LC_BRAND_NAME}_$1.$2.$3_macOSX_SymbolTables

popd
return 0
