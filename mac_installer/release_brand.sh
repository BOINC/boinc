#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
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

##
# Script to create a branded installer such as World Community Grid
#
## Note 1: This script currently does not have support for "BOINC+VirtualBox.mpkg"
## and "BOINC + VirtualBox.mpkg"
##

## Usage:
## First a brand description file must be created for the desired brand with the variable below defined
##  SHORTBRANDNAME="wcgrid"                             ##Used to name the branded files and directories in the BOINC_Installer directory
##  LONGBRANDNAME="World Community Grid"                ##Used to name the installer package
##  MANAGERAPPNAME="World Community Grid"               ##The name of the branded manager, replaces BOINC Manager
##  MANAGERICON="WCGridMgr"                             ##The icon for the branded manager, replaces BOINCMgr.icns
##  SSAVERAPPNAME="World Community Grid"                ##The branded screen saver name
##  SSAVERPREVIEW="wcgrid.jpg"                          ##The image used as the preview for the screen saver in system preferences
##  SSAVERTHUMBNAIL="wcgrid-thumbnail"                  ##The image used as the thumbnail for the screen saver in system preferences
##  SSAVERLOGO="wcg_ss_logo.jpg"                        ##The image used in the generic screen saver
##  SKINDIR="World Community Grid"                      ##The branded skin name to use
##  UNINSTALLERAPPNAME="Uninstall World Community Grid" ##The name of the uninstaller app
##  UNINSTALLERICON="WCGridUninstaller.icns"            ##The icon for the branded uninstaller
##  INSTALLERAPPNAME="World Community Grid Installer"   ##The name of the installer app
##  INSTALLERICON="WCGridInstaller.icns"                ##The icon for the branded installer
##  READMEFILE="WCGrid-ReadMe.rtf"                      ##The branded readme file
##  BRANDING_INFO="BrandId=4"                           ##Info to write into the branding file
##  NEWVERSIONCHECKDIR="WCG"                            ##Where to get nvc_config.xml, empty string if none
##
##  This script expects the skin to be at "./clientgui/skins/${SKINDIR}"
##  This script expects the nvc_config.xml file (if any) to be at
##  "./win_build/installerv2/redist/${NEWVERSIONCHECKDIR}/nvc_config.xml"
##
## NOTE: This script requires Mac OS 10.6 or later, and uses XCode developer
##   tools.  So you must have installed XCode Developer Tools on the Mac
##   before running this script.
##
## If you wish to code sign the client, manager, installer and uninstaller,
## create a file ~/BOINCCodeSignIdentities.txt whose first line is the
## application code signing identity and whose second line is the installer
## code signing identity.
## If you wish to also code sign the installer package, add a second line
## to ~/BOINCCodeSignIdentities.txt with the installer code signing identity.
##
## cd to the root directory of the boinc tree, for example:
##     cd <path>/boinc
##
## Then invoke this script with the three parts of version number as arguments and the full path
## to the brand description file. For example, if the version is 3.2.1:
##     source ./mac_installer/release_brand.sh 3 2 1 ./mac_installer/WCGridInstaller.environment
##
## This will create a directory "BOINC_Installer" in the parent directory of
## the current directory
##
## For testing only, you can use the development build by adding a fifth argument -dev
## For example, if the version is 3.2.1:
##     source /mac_installer/release_brand.sh 3 2 1 ./mac_installer/WCGridInstaller.environment -dev

## As of OS 10.14 Mojave, Apple has introduced a new level of security which
## Apple calls "notarization". Under OS 10.14, the only difference is that
## Gatekeeper adds the sentence "Apple checked it for malicious software and
## found none." However, Apple has warned: "In an upcoming release of macOS,
## Gatekeeper will require Developer ID–signed software to be notarized by
## Apple."
##
## To notarize the installer and uninstaller:
## NOTE: Do not use your normal Apple ID password. You must create an
## app-specific password at https://appleid.apple.com/account/manage.
##
## NOTE: in the following instructions, subsitute:
##   * the 3 part version numberfor x.y.z
##   * the ${SHORTBRANDNAME} for $SBN (for example, wcgrid for World Community Grid)
##   * the architecture (usually "universal" for $arch)
##   so substitute the quoted full path for ".../$SBN_x.y.z_macOSX_$arch"
## - Use the command line tools in Xcode 13 or later
## - Provide valid application & installer code signing identities as above
## - In the instructions below, substitute the appropriate architcture for $arch
##     (either x86_64, arm64 or universal)
## - In Terminal:
##  $ xcrun notarytool submit ".../$SBN_x.y.z_macOSX_$arch.zip" --apple-id {your_Apple_ID} --password {password} --team-id {your_team_ID) --wait
##
## - If the notarytool submit request was approved, attach tickets to top level applications as follows:
## NOTE: Stapling the original files never works. We must rename the original
##       directory and recreate it from the zip file we just submitted
##  $ mv ".../$SBN.y.z_macOSX_$arch" ".../$SBN.y.z_macOSX_$arch-orig"
##  $ open ".../$SBN.y.z_macOSX_$arch.zip"
##  $ xcrun stapler staple ".../$SBN.y.z_macOSX_$arch/${INSTALLERAPPNAME}.app"
##  $ xcrun stapler staple {path to ".../$SBN.y.z_macOSX_$arch.zip/extras/${UNINSTALLERAPPNAME}.app"
## - delete or rename the original ".../$SBN.y.z_macOSX_$arch.zip" file
## - Run this ditto command again to create a new zip archive containing
##   the updated (notarized) Installer app and Uninstaller app:
##  $ ditto -ck --sequesterRsrc --keepParent ".../$SBN.y.z_macOSX_$arch" ".../$SBN.y.z_macOSX_$arch.zip"
##
## Then notarize the bare-core (apple-darwin) release as follows:
##  $ xcrun notarytool submit ".../$SBN.y.z_$arch-apple-darwin.dmg" --apple-id {your_Apple_ID} --password {password} --team-id {your_team_ID) --wait
##  $ xcrun altool --notarization-info {UUID from last step} -u {userID} -p {password}
##
## - If the notarize-app request was approved, attach a ticket to the disk image:
## NOTE: Stapling the original files never works. We must rename the original
##       disk image we just submitted and make a copy of it
##  $ mv ".../$SBN.y.z_$arch-apple-darwin.dmg" ".../$SBN.y.z_$arch-apple-darwin-orig.dmg"
##  $ cp ".../$SBN.y.z_$arch-apple-darwin-orig.dmg" ".../$SBN.y.z_$arch-apple-darwin.dmg"
##  $ xcrun stapler staple ".../$SBN.y.z_$arch-apple-darwin.dmg"
##
## - for more information:
##  $ xcrun notarytool --help
##  $ man stapler
##
## TODO: Add code to optionally automate notarization either in this script or
## TODO: in a separate script. Perhaps adapt notarization and stapler code from
## TODO: <https://github.com/smittytone/scripts/blob/master/packcli.zsh>
##

if [ $# -lt 4 ]; then
echo "Usage:"
echo "   cd <path>/boinc"
echo "   source path_to_this_script major_version minor_version revision_number brand_description_file [-dev]"
echo "example:"
echo "   source ./mac_installer/release_brand.sh 7 11 0 ./mac_installer/WCGridInstaller.environment"
return 1
fi

#source brand description file
if [ ! -f ${4} ]; then
    echo Brand description file ${4} not found
    return 1
fi
. ${4}

BOINCPath=$PWD

##DarwinVersion=`uname -r`;
##DarwinMajorVersion=`echo $DarwinVersion | sed 's/\([0-9]*\)[.].*/\1/' `;
# DarwinMinorVersion=`echo $version | sed 's/[0-9]*[.]\([0-9]*\).*/\1/' `;
#
# echo "major = $DarwinMajorVersion"
# echo "minor = $DarwinMinorVersion"
#
# Darwin version 11.x.y corresponds to OS 10.7.x
# Darwin version 10.x.y corresponds to OS 10.6.x
# Darwin version 8.x.y corresponds to OS 10.4.x
# Darwin version 7.x.y corresponds to OS 10.3.x
# Darwin version 6.x corresponds to OS 10.2.x

arch="x86_64"

# XCode 3.x and 4.x use different paths for their build products.
# Our scripts in XCode's script build phase write those paths to
# files to help this release script find the build products.
if [ "$5" = "-dev" ]; then
    exec 7<"mac_build/Build_Development_Dir"
    read -u 7 BUILDPATH
else
    exec 7<"mac_build/Build_Deployment_Dir"
    read -u 7 BUILDPATH
fi

arch="x86_64"

Products_Have_x86_64="no"
Products_Have_arm64="no"
cd "${BUILDPATH}"
lipo "BOINCManager.app/Contents/MacOS/BOINCManager" -verify_arch x86_64
if [ $? -eq 0 ]; then Products_Have_x86_64="yes"; fi
lipo "BOINCManager.app/Contents/MacOS/BOINCManager" -verify_arch arm64
if [ $? -eq 0 ]; then Products_Have_arm64="yes"; fi
if [ $Products_Have_x86_64 = "no" ] && [ $Products_Have_arm64 = "no" ]; then
    echo "ERROR: could not determine architecture of BOINC Manager"
fi
if [ $Products_Have_arm64 = "yes" ]; then
    if [ $Products_Have_x86_64 = "yes" ]; then
        arch="universal"
    else
        arch="arm64"
    fi
fi

for Executable in "boinc" "boinccmd" "switcher" "setprojectgrp" "boincscr" "Fix_BOINC_Users" "Run_Podman" "BOINCSaver.saver/Contents/MacOS/BOINCSaver" "BOINCSaver.saver/Contents/Resources/gfx_switcher" "BOINCSaver.saver/Contents/Resources/gfx_cleanup" "BOINCSaver.saver/Contents/Resources/gfx_ss_bridge" "Uninstall BOINC.app/Contents/MacOS/Uninstall BOINC" "BOINC Installer.app/Contents/MacOS/BOINC Installer" "PostInstall.app/Contents/MacOS/PostInstall" "BOINC_Finish_Install.app/Contents/MacOS/BOINC_Finish_Install" "AddRemoveUser"
do
    Have_x86_64="no"
    Have_arm64="no"
    lipo "${Executable}" -verify_arch x86_64
    if [ $? -eq 0 ]; then Have_x86_64="yes"; fi
    lipo "${Executable}" -verify_arch arm64
    if [ $? -eq 0 ]; then Have_arm64="yes"; fi

    if [ $Have_x86_64 != $Products_Have_x86_64 ] || [ $Have_arm64 != $Products_Have_arm64 ]; then
        echo "ERROR: Architecture mismatch: BOINC Manager and " "${Executable}"
        return 1
    fi
done

cd "${BOINCPath}"

sudo rm -dfR ../BOINC_Installer/Installer\ Resources/
sudo rm -dfR ../BOINC_Installer/Installer\ Scripts/
sudo rm -dfR ../BOINC_Installer/Pkg_Root
sudo rm -dfR ../BOINC_Installer/locale
sudo rm -dfR ../BOINC_Installer/Installer\ templates
sudo rm -dfR ../BOINC_Installer/expandedVBox

mkdir -p ../BOINC_Installer/Installer\ Resources/
mkdir -p ../BOINC_Installer/Installer\ Scripts/
mkdir -p ../BOINC_Installer/Installer\ templates

cp -fp mac_installer/License.rtf ../BOINC_Installer/Installer\ Resources/
cp -fp mac_installer/${READMEFILE} ../BOINC_Installer/Installer\ Resources/ReadMe.rtf

cp -fp mac_installer/complist.plist ../BOINC_Installer/Installer\ templates/complist.plist
sed -i "" s/BOINCManager.app/"${MANAGERAPPNAME}.app"/g ../BOINC_Installer/Installer\ templates/complist.plist
sed -i "" s/BOINCSaver.saver/"${SSAVERAPPNAME}.saver"/g ../BOINC_Installer/Installer\ templates/complist.plist


cp -fp mac_installer/myDistribution ../BOINC_Installer/Installer\ templates/myDistribution
sed -i "" s/BOINCManager.app/"${MANAGERAPPNAME}.app"/g ../BOINC_Installer/Installer\ templates/myDistribution
sed -i "" s/BOINCSaver.saver/"${SSAVERAPPNAME}.saver"/g ../BOINC_Installer/Installer\ templates/myDistribution
sed -i "" s/"BOINC Manager"/"${MANAGERAPPNAME}"/g ../BOINC_Installer/Installer\ templates/myDistribution


# Update version number
sed -i "" s/"<VER_NUM>"/"$1.$2.$3"/g ../BOINC_Installer/Installer\ Resources/ReadMe.rtf
sed -i "" s/"x.y.z"/"$1.$2.$3"/g ../BOINC_Installer/Installer\ templates/myDistribution

# Fix hostArchitectures option
has_x86_64="no"
has_arm64="no"
client_archs=`lipo -info "${BUILDPATH}/boinc"`
if [[ "${client_archs}" = *"x86_64"* ]]; then has_x86_64="yes"; fi
if [[ "${client_archs}" = *"arm64"* ]]; then has_arm64="yes"; fi

if [ $has_x86_64 = "no" ]; then
    sed -i "" s/"x86_64,arm64"/"arm64"/g ../BOINC_Installer/Installer\ templates/myDistribution
else
    if [ $has_arm64 = "no" ]; then
        sed -i "" s/"x86_64,arm64"/"x86_64"/g ../BOINC_Installer/Installer\ templates/myDistribution
    fi
fi

#### We don't customize BOINC Data directory name for branding
cp -fp mac_installer/preinstall ../BOINC_Installer/Installer\ Scripts/
cp -fp mac_installer/preinstall ../BOINC_Installer/Installer\ Scripts/preupgrade
cp -fp mac_installer/postinstall ../BOINC_Installer/Installer\ Scripts/
cp -fp mac_installer/postupgrade ../BOINC_Installer/Installer\ Scripts/
mkdir -p ../BOINC_Installer/Pkg_Root
mkdir -p ../BOINC_Installer/Pkg_Root/Applications
mkdir -p ../BOINC_Installer/Pkg_Root/Library
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Screen\ Savers
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/locale
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/switcher
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/skins
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ podman

# We must create virtualbox directory so installer will set up its
# ownership and permissions correctly, because vboxwrapper won't
# have permission to set owner to boinc_master.
#mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/projects/virtualbox

cp -fpRL "${BUILDPATH}/switcher" ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/switcher/
cp -fpRL "${BUILDPATH}/setprojectgrp" ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/switcher/

cd "${BOINCPath}/clientgui/skins"
cp -fpRL Default ../../../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/skins/
cp -fpRL "${SKINDIR}" ../../../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/skins/
cd "${BOINCPath}"

cp -fp win_build/installerv2/redist/all_projects_list.xml ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/installer_projects_list.xml

if [ -n "${NEWVERSIONCHECKDIR}" ]; then
    cp -fp "win_build/installerv2/redist/${NEWVERSIONCHECKDIR}/nvc_config.xml" ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/nvc_config.xml
fi

cp -fp clientscr/res/boinc_logo_black.jpg ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/
cp -fp api/ttf/liberation-fonts-ttf-2.00.0/LiberationSans-Regular.ttf ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/LiberationSans-Regular.ttf
cp -fp clientscr/ss_config.xml ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/
cp -fpRL "${BUILDPATH}/boincscr" ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/
cp -fpRL "${BUILDPATH}/detect_rosetta_cpu" ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/
cp -fpRL "${BUILDPATH}/Fix_BOINC_Users" ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/
cp -fpRL "${BUILDPATH}/Run_Podman" ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/

cp -fpRL "${BUILDPATH}/BOINCManager.app/." "../BOINC_Installer/Pkg_Root/Library/Application Support/${MANAGERAPPNAME}.app/"
sed -i "" s/BOINCManager/"${MANAGERAPPNAME}"/g "../BOINC_Installer/Pkg_Root/Library/Application Support/${MANAGERAPPNAME}.app/Contents/Info.plist"
sed -i "" s/BOINCMgr/"${MANAGERICON}"/g "../BOINC_Installer/Pkg_Root/Library/Application Support/${MANAGERAPPNAME}.app/Contents/Info.plist"
mv "../BOINC_Installer/Pkg_Root/Library/Application Support/${MANAGERAPPNAME}.app/Contents/MacOS/BOINCManager" "../BOINC_Installer/Pkg_Root/Library/Application Support/${MANAGERAPPNAME}.app/Contents/MacOS/${MANAGERAPPNAME}"
cp -fpRL clientgui/res/${MANAGERICON}.icns "../BOINC_Installer/Pkg_Root/Library/Application Support/${MANAGERAPPNAME}.app/Contents/Resources/"
rm -rf "../BOINC_Installer/Pkg_Root/Library/Application Support/${MANAGERAPPNAME}.app/Contents/Resources/BOINCMgr.icns"
sed -i "" s/"BOINC Manager"/"${MANAGERAPPNAME}"/g "../BOINC_Installer/Pkg_Root/Library/Application Support/${MANAGERAPPNAME}.app/Contents/Resources/English.lproj/InfoPlist.strings"

echo ${BRANDING_INFO} > "../BOINC_Installer/Pkg_Root/Library/Application Support/${MANAGERAPPNAME}.app/Contents/Resources/Branding"
echo ${BRANDING_INFO} > ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/Branding

cp -fpRL "${BUILDPATH}/BOINCSaver.saver/." "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/"
mv "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/Contents/MacOS/BOINCSaver" "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/Contents/MacOS/${SSAVERAPPNAME}"

sed -i "" s/BOINCSaver/"${SSAVERAPPNAME}"/g "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/Contents/Info.plist"
cp -fpRL clientscr/res/${SSAVERPREVIEW} "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/Contents/Resources/boinc.jpg"
cp -fpRL clientscr/res/${SSAVERTHUMBNAIL}.png "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/Contents/Resources/thumbnail.png"
cp -fpRL clientscr/res/${SSAVERTHUMBNAIL}\@2x.png "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/Contents/Resources/thumbnail@2x.png"
cp -fpRL clientscr/res/${SSAVERLOGO} "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/Contents/Resources/boinc_ss_logo.png"
cp -fpRL clientscr/res/${SSAVERLOGO} ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/
sed -i "" s/BOINC/"${SSAVERAPPNAME}"/g "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/Contents/Resources/English.lproj/InfoPlist.strings"


## Copy the localization files into the installer tree
## Old way copies CVS and *.po files which are not needed
## cp -fpRL locale/ ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/locale
## sudo rm -dfR ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/locale/CVS
## New way copies only *.mo files (adapted from boinc/sea/make-tar.sh)
find locale -name '*.mo' | cut -d '/' -f 2 | awk '{print "\"../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/locale/"$0"\""}' | xargs mkdir -p
find locale -name '*.mo' | cut -d '/' -f 2,3 | awk '{print "cp \"locale/"$0"\" \"../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/locale/"$0"\""}' | bash

## Fix up ownership and permissions
sudo chown -R root:admin ../BOINC_Installer/Pkg_Root/*
sudo chmod -R u+rw,g+rw,o+r-w ../BOINC_Installer/Pkg_Root/*
sudo chmod 1775 ../BOINC_Installer/Pkg_Root/Library

sudo chown -R 501:admin ../BOINC_Installer/Pkg_Root/Library/Application\ Support/*
sudo chmod -R u+rw,g+r-w,o+r-w ../BOINC_Installer/Pkg_Root/Library/Application\ Support/*

sudo chown -R root:admin ../BOINC_Installer/Installer\ Resources/*
sudo chown -R root:admin ../BOINC_Installer/Installer\ Scripts/*
sudo chmod -R u+rw,g+r-w,o+r-w ../BOINC_Installer/Installer\ Resources/*
sudo chmod -R u+rw,g+r-w,o+r-w ../BOINC_Installer/Installer\ Scripts/*

sudo rm -dfR ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/

mkdir -p ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/
mkdir -p ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch
mkdir -p ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras
mkdir -p ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin
mkdir -p ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_SymbolTables

cp -fp ../BOINC_Installer/Installer\ Resources/ReadMe.rtf ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch
sudo chown -R 501:admin ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/ReadMe.rtf
sudo chmod -R 644 ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/ReadMe.rtf

cp -fp COPYING ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/COPYING.txt
sudo chown -R 501:admin ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/COPYING.txt
sudo chmod -R 644 ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/COPYING.txt

cp -fp COPYING.LESSER ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/COPYING.LESSER.txt
sudo chown -R 501:admin ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/COPYING.LESSER.txt
sudo chmod -R 644 ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/COPYING.LESSER.txt

cp -fp COPYRIGHT ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/COPYRIGHT.txt
sudo chown -R 501:admin ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/COPYRIGHT.txt
sudo chmod -R 644 ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/COPYRIGHT.txt

cp -fpRL "${BUILDPATH}/Uninstall BOINC.app/." "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/"

# Copy the localization files for the uninstaller into its bundle
find locale -name 'BOINC-Setup.mo' | cut -d '/' -f 2 | awk '{print "\"../BOINC_Installer/locale/"$0"\""}' | xargs mkdir -p

find locale -name 'BOINC-Setup.mo' | cut -d '/' -f 2,3 | awk '{print "cp \"locale/"$0"\" \"../BOINC_Installer/locale/"$0"\""}' | bash

sudo cp -fpRL ../BOINC_Installer/locale "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources"
sed -i "" s/"Uninstall BOINC"/"${UNINSTALLERAPPNAME}"/g "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Info.plist"
sed -i "" s/"MacUninstaller"/"${UNINSTALLERICON}"/g "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Info.plist"
mv "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/MacOS/Uninstall BOINC" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/MacOS/${UNINSTALLERAPPNAME}"
sed -i "" s/"Uninstall BOINC"/"${UNINSTALLERAPPNAME}"/g "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/English.lproj/InfoPlist.strings"
cp -fpRL ./clientgui/res/${UNINSTALLERICON}.icns "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/"
rm -rf "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/MacUninstaller.icns"

echo ${BRANDING_INFO} > "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/Branding"

sudo chown -R root:admin "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app"
sudo chmod -R u+r-w,g+r-w,o+r-w "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app"

# Copy the installer wrapper application "BOINC Installer.app"
cp -fpRL "${BUILDPATH}/BOINC Installer.app/." "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app/"
sed -i "" s/"BOINC Installer"/"${INSTALLERAPPNAME}"/g "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app/Contents/Info.plist"
sed -i "" s/"MacInstaller"/"${INSTALLERICON}"/g "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app/Contents/Info.plist"
cp -fpRL ./clientgui/res/${INSTALLERICON}.icns "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app/Contents/Resources/"
rm -rf "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app/Contents/Resources/MacInstaller.icns"
mv "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app/Contents/MacOS/BOINC Installer" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app/Contents/MacOS/${INSTALLERAPPNAME}"
sed -i "" s/"BOINC Installer"/"${INSTALLERAPPNAME}"/g "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app/Contents/Resources/English.lproj/InfoPlist.strings"

# Copy BOINC_Finish_Install.app into BOINC Data folder
cp -fpR "${BUILDPATH}/BOINC_Finish_Install.app" "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/"

# Rename BOINC_Finish_Install.app
sudo mv "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/BOINC_Finish_Install.app" "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/${LONGBRANDNAME}_Finish_Install.app"

# Change executable name of BOINC_Finish_Install.app to match app name
sudo mv "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/${LONGBRANDNAME}_Finish_Install.app/Contents/MacOS/BOINC_Finish_Install" "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/${LONGBRANDNAME}_Finish_Install.app/Contents/MacOS/${LONGBRANDNAME}_Finish_Install"

sudo plutil -replace CFBundleExecutable -string "${LONGBRANDNAME}_Finish_Install" "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/${LONGBRANDNAME}_Finish_Install.app/Contents/Info.plist"

# Replace icon reference in Info.plist file of ${LONGBRANDNAME}_Finish_Install.app
sudo plutil -replace CFBundleIconFile -string "${INSTALLERICON}.icns" "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/${LONGBRANDNAME}_Finish_Install.app/Contents/Info.plist"

# Replace icon in ${LONGBRANDNAME}_Finish_Install.app
sudo rm -dfR "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/${LONGBRANDNAME}_Finish_Install.app/Contents/Resources/MacInstaller.icns"

sudo cp -fpRL "./clientgui/res/${INSTALLERICON}.icns" "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/${LONGBRANDNAME}_Finish_Install.app/Contents/Resources/"


cp -fpR "${BUILDPATH}/PostInstall.app" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app/Contents/Resources"

# Change BOINC_Finish_Install.app embedded in uninstaller to BOINC_Finish_Uninstall.app
sudo mv "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/BOINC_Finish_Install.app" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/${LONGBRANDNAME}_Finish_Uninstall.app"

# Change executable name of BOINC_Finish_Uninstall.app to match app name
sudo mv "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/${LONGBRANDNAME}_Finish_Uninstall.app/Contents/MacOS/BOINC_Finish_Install" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/${LONGBRANDNAME}_Finish_Uninstall.app/Contents/MacOS/${LONGBRANDNAME}_Finish_Uninstall"

# Fix version number in Info.plist file of BOINC_Finish_Uninstall.app
sudo plutil -replace CFBundleVersion -string `plutil -extract CFBundleVersion raw "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Info.plist"` "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/${LONGBRANDNAME}_Finish_Uninstall.app/Contents/Info.plist"

sudo plutil -remove CFBundleShortVersionString "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/${LONGBRANDNAME}_Finish_Uninstall.app/Contents/Info.plist"

# Fix bundle name in Info.plist file of BOINC_Finish_Uninstall.app
sudo plutil -replace CFBundleName -string "BOINC_Finish_Uninstall" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/${LONGBRANDNAME}_Finish_Uninstall.app/Contents/Info.plist"

sudo plutil -replace CFBundleExecutable -string "${LONGBRANDNAME}_Finish_Uninstall" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/${LONGBRANDNAME}_Finish_Uninstall.app/Contents/Info.plist"

# Change Bundle ID of BOINC_Finish_Uninstall.app
###sudo plutil -replace CFBundleIdentifier -string edu.berkeley.boinc.finish-uninstall "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/${LONGBRANDNAME}_Finish_Uninstall.app/Contents/Info.plist"

# Replace icon in ${LONGBRANDNAME}_Finish_Uninstall.app
# Replace MacInstaller.icns with ${UNINSTALLERICON}.icns, but renamed MacInstaller.icns
###cp -fpRL ./clientgui/res/${UNINSTALLERICON}.icns "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/${LONGBRANDNAME}_Finish_Uninstall.app/Contents/Resources/MacInstaller.icns"

# Replace icon reference in Info.plist file of ${LONGBRANDNAME}_Finish_Uninstall.app
sudo plutil -replace CFBundleIconFile -string "${UNINSTALLERICON}.icns" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/${LONGBRANDNAME}_Finish_Uninstall.app/Contents/Info.plist"

# Replace icon in ${LONGBRANDNAME}_Finish_Uninstall.app
sudo rm -dfR "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/${LONGBRANDNAME}_Finish_Uninstall.app/Contents/Resources/MacInstaller.icns"

sudo cp -fpRL "./clientgui/res/${UNINSTALLERICON}.icns" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/${LONGBRANDNAME}_Finish_Uninstall.app/Contents/Resources/"

echo ${BRANDING_INFO} > "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app/Contents/Resources/PostInstall.app/Contents/Resources/Branding"

# Replace MacInstaller.icns with ${INSTALLERICON}.icns, but renamed MacInstaller.icns
cp -fpRL ./clientgui/res/${INSTALLERICON}.icns "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app/Contents/Resources/PostInstall.app/Contents/Resources/MacInstaller.icns"


## If you wish to code sign the client, manager, installer and uninstaller,
## create a file ~/BOINCCodeSignIdentities.txt whose first line is the
## application code signing identity and whose second line is the installer
## code signing identity.
## If you wish to also code sign the installer package, add a second line
## to ~/BOINCCodeSignIdentities.txt with the installer code signing identity.
##
## Code signing using a registered Apple Developer ID is necessary for GateKeeper
## with default settings to allow running downloaded applications under OS 10.8
## Although code signing the installer application is sufficient to satisfy
## GateKeeper, OS X's software firewall can interfere with RPCs between the
## client and manager.  Signing them may make this less likely to be a problem.
if [ -e "${HOME}/BOINCCodeSignIdentities.txt" ]; then
    exec 8<"${HOME}/BOINCCodeSignIdentities.txt"
    read APPSIGNINGIDENTITY <&8
    read INSTALLERSIGNINGIDENTITY <&8

    # Code Sign the switcher utility if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/switcher/switcher"

    # Code Sign the setprojectgrp utility if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/switcher/setprojectgrp"

    # Code Sign the boincscr graphics app if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/boincscr"

    # Code Sign the detect_rosetta_cpu helper app if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/detect_rosetta_cpu"

    # Code Sign the Fix_BOINC_Users helper app if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/Fix_BOINC_Users"

    # Code Sign the Run_Podman helper app if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/Run_Podman"

    # Code Sign the gfx_switcher utility embedded in BOINC screensaver if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/Contents/Resources/gfx_switcher"

    # Code Sign the gfx_cleanup utility embedded in BOINC screensaver if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/Contents/Resources/gfx_cleanup"

    # Code Sign the gfx_ss_bridge utility embedded in BOINC screensaver if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/Contents/Resources/gfx_ss_bridge"

    # Code Sign the BOINC screensaver code for OS 10.8 and later if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/"

    # Code Sign the BOINC client embedded in the Manager if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Application Support/${MANAGERAPPNAME}.app/Contents/Resources/boinc"

    # Code Sign the BOINC Manager if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Application Support/${MANAGERAPPNAME}.app"

    # Code Sign the PostInstall app embedded in the BOINC installer app if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app/Contents/Resources/PostInstall.app"

    # Code Sign BOINC_Finish_Uninstall app embedded in BOINC uninstaller app if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/${LONGBRANDNAME}_Finish_Uninstall.app"

    # Code Sign the BOINC uninstaller app if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app"

    # Code Sign the BOINC_Finish_Install.app in the BOINC Data folder
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}"     "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/${LONGBRANDNAME}_Finish_Install.app"
fi

# Build the installer package inside the wrapper application's bundle

cd "../BOINC_Installer/Installer templates"

pkgbuild --quiet --scripts "../Installer Scripts" --ownership recommended --identifier edu.berkeley.boinc --root "../Pkg_Root" --component-plist "./complist.plist" "./BOINC.pkg"

if [ -n "${INSTALLERSIGNINGIDENTITY}" ]; then
    productbuild --sign "${INSTALLERSIGNINGIDENTITY}" --quiet --resources "../Installer Resources/" --version "${MANAGERAPPNAME} $1.$2.$3" --distribution "./myDistribution" "../New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app/Contents/Resources/${LONGBRANDNAME}.pkg"
else
    productbuild --quiet --resources "../Installer Resources/" --version "${MANAGERAPPNAME} $1.$2.$3" --distribution "./myDistribution" "../New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app/Contents/Resources/${LONGBRANDNAME}.pkg"
fi
cd "${BOINCPath}"

# Build the stand-alone client distribution
cp -fpRL mac_build/Mac_SA_Insecure.sh ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/
cp -fpRL mac_build/Mac_SA_Secure.sh ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/
cp -fpRL COPYING ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/COPYING.txt
cp -fpRL COPYING.LESSER ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/COPYING.LESSER.txt
cp -fpRL COPYRIGHT ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/COPYRIGHT.txt
cp -fp mac_installer/License.rtf ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/
sudo chown -R 501:admin ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/*
sudo chmod -R 644 ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/*

mkdir -p ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir
cp -fpRL "${BUILDPATH}/boinc" ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/
cp -fpRL "${BUILDPATH}/boinccmd" ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/
cp -fpRL "${BUILDPATH}/detect_rosetta_cpu" ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/

mkdir -p ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/switcher
cp -fpRL "${BUILDPATH}/switcher" ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/switcher/
cp -fpRL "${BUILDPATH}/setprojectgrp" ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/switcher/

sudo chown -R root:admin ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/*
sudo chmod -R u+rw-s,g+r-ws,o+r-w ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/*

cp -fpRL "${BUILDPATH}/boinc.dSYM" ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_SymbolTables/
cp -fpRL "${BUILDPATH}/BOINCManager.app.dSYM" ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_SymbolTables/"${MANAGERAPPNAME}.app.dSYM"

## If you wish to code sign the installer and uninstaller, create a file
## ~/BOINCCodeSignIdentities.txt whose first line is the code signing identity
##
## Code signing using a registered Apple Developer ID is necessary for GateKeeper
## with default settings to allow running downloaded applications under OS 10.8
if [ -n "${APPSIGNINGIDENTITY}" ]; then
    # Code Sign the BOINC installer application if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app"

    # Code Sign the stand-alone bare core boinc client if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/boinc"

    # Code Sign detect_rosetta_cpu for the stand-alone boinc client if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/detect_rosetta_cpu"

    # Code Sign setprojectgrp for the stand-alone boinc client if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/switcher/setprojectgrp"

    # Code Sign switcher for the stand-alone boinc client if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/switcher/switcher"

    if [ $arch = "universal" ]; then
        # Workaround for code signing problem under Xcode 12.2:
        # Code sign each architecture separately then combine into a uiversal binary
        lipo "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/boinccmd" -thin x86_64 -output "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/boinccmd-x86_64"

        lipo "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/boinccmd" -thin arm64 -output "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/boinccmd-arm64"

        sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/boinccmd-x86_64"

        sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/boinccmd-arm64"

        rm -f "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/boinccmd"

        lipo "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/boinccmd-x86_64" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/boinccmd-arm64" -create -output "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/boinccmd"

        rm -f "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/boinccmd-x86_64"

        rm -f "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/boinccmd-arm64"

   else
        # Code Sign boinccmd for the stand-alone boinc client if we have a signing identity
        sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/boinccmd"
    fi
fi

cd ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3

## Make everything in directory user-writable so project web code using auto-attach
## can delete it after inflating, modifying installer name and recompressing it.
sudo chmod -R u+w ./${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch

## Use ditto instead of zip utility to preserve resource forks and Finder attributes (custom icon, hide extension)
ditto -ck --sequesterRsrc --keepParent ${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch ${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch.zip
ditto -ck --sequesterRsrc --keepParent ${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin ${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin.zip
ditto -ck --sequesterRsrc --keepParent ${SHORTBRANDNAME}_$1.$2.$3_macOSX_SymbolTables ${SHORTBRANDNAME}_$1.$2.$3_macOSX_SymbolTables.zip

sudo hdiutil create -srcfolder ${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin -ov -format UDZO ${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin.dmg

#popd
cd "${BOINCPath}"

sudo rm -dfR ../BOINC_Installer/Installer\ Resources/
sudo rm -dfR ../BOINC_Installer/Installer\ Scripts/
sudo rm -dfR ../BOINC_Installer/Pkg_Root
sudo rm -dfR ../BOINC_Installer/locale
sudo rm -dfR ../BOINC_Installer/Installer\ templates
sudo rm -dfR ../BOINC_Installer/expandedVBox

return 0
