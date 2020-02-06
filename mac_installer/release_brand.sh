#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2019 University of California
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
## - Use the command line tools in Xcode 10 or later
## - Provide valid application & installer code signing identities as above
## - In Terminal":
##  $ xcrun altool --notarize-app -t osx -f {path to ...macOSX_x86_64.zip} --primary-bundle-id edu.berkeley.boinc.Installer -u {userID} -p {password}
## - After a few minutes, check whether the notarize-app request succeeded:
##  $ xcrun altool --notarization-info {UUID from last step} -u {userID} -p {password}
## - If the notarize-app request succeeded, attach tickets to top level applications:
##  $ xcrun stapler staple {path to "...macOSX_x86_64/${INSTALLERAPPNAME}.app"}
##  $ xcrun stapler staple {path to "...macOSX_x86_64/extras/${UNINSTALLERAPPNAME}.app"}
## - delete or rename the original ...macOSX_x86_64.zip}
## - Run this ditto command again to create a new ...macOSX_x86_64.zip containing 
##   the updated (notarized) ${INSTALLERAPPNAME}.app and ${UNINSTALLERAPPNAME}.app:
##  $ ditto -ck --sequesterRsrc --keepParent ${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch ${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch.zip
## - Note: if you are running stapler under OS 10.13 and get an error 68, the local CRL
##   cache may have become corrupted. You can resolve this by either running stapler
##   under MacOS 10.14 Mojave or by running this command under OS 10.13:
##     $ sudo killall -9 trustd; sudo rm /Library/Keychains/crls/valid.sqlite3 
## - for more information:
##  $ xcrun altool --help
##  $ man stapler

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

DarwinVersion=`uname -r`;
DarwinMajorVersion=`echo $DarwinVersion | sed 's/\([0-9]*\)[.].*/\1/' `;
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

if [ "$DarwinMajorVersion" -gt 10 ]; then
    # XCode 4.1 on OS 10.7 builds only Intel binaries
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

else
    # XCode 3.2 on OS 10.6 does build Intel and PowerPC Universal binaries
    arch="universal"

    # XCode 3.x and 4.x use different paths for their build products.
    if [ "$5" = "-dev" ]; then
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
fi

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

cp -fp curl/ca-bundle.crt ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/

cp -fp win_build/installerv2/redist/all_projects_list.xml ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/installer_projects_list.xml

if [ -n "${NEWVERSIONCHECKDIR}" ]; then
    cp -fp "win_build/installerv2/redist/${NEWVERSIONCHECKDIR}/nvc_config.xml" ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/nvc_config.xml
fi

cp -fp clientscr/res/boinc_logo_black.jpg ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/
cp -fp api/ttf/liberation-fonts-ttf-2.00.0/LiberationSans-Regular.ttf ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/LiberationSans-Regular.ttf
cp -fp clientscr/ss_config.xml ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/
cp -fpRL "${BUILDPATH}/boincscr" ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/

cp -fpRL "${BUILDPATH}/BOINCManager.app/." "../BOINC_Installer/Pkg_Root/Applications/${MANAGERAPPNAME}.app/"
sed -i "" s/BOINCManager/"${MANAGERAPPNAME}"/g "../BOINC_Installer/Pkg_Root/Applications/${MANAGERAPPNAME}.app/Contents/Info.plist"
sed -i "" s/BOINCMgr/"${MANAGERICON}"/g "../BOINC_Installer/Pkg_Root/Applications/${MANAGERAPPNAME}.app/Contents/Info.plist"
mv "../BOINC_Installer/Pkg_Root/Applications/${MANAGERAPPNAME}.app/Contents/MacOS/BOINCManager" "../BOINC_Installer/Pkg_Root/Applications/${MANAGERAPPNAME}.app/Contents/MacOS/${MANAGERAPPNAME}"
cp -fpRL clientgui/res/${MANAGERICON}.icns "../BOINC_Installer/Pkg_Root/Applications/${MANAGERAPPNAME}.app/Contents/Resources/"
rm -rf "../BOINC_Installer/Pkg_Root/Applications/${MANAGERAPPNAME}.app/Contents/Resources/BOINCMgr.icns"
sed -i "" s/"BOINC Manager"/"${MANAGERAPPNAME}"/g "../BOINC_Installer/Pkg_Root/Applications/${MANAGERAPPNAME}.app/Contents/Resources/English.lproj/InfoPlist.strings"

echo ${BRANDING_INFO} > "../BOINC_Installer/Pkg_Root/Applications/${MANAGERAPPNAME}.app/Contents/Resources/Branding"
echo ${BRANDING_INFO} > ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/Branding



## OS 10.6 and OS10.7 require screensavers built with Garbage Collection, but Xcode 5.0.2
## was the last version of Xcode which supported building with Garbage Collection, so we
## have saved the screensaver executable with GC as a binary. Add it to the screen saver
## passed to the BOINC installer. At install time, he BOINC installer will select the
## correct binary for the version of OS X and delete the other one. This script assumes
## that $BUILDPATH/BOINCSaver.saver was built to use Automatic Reference Counting (ARC)
## and not built to use GC.

cp -fpRL "${BUILDPATH}/BOINCSaver.saver/." "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/"
ditto -xk ./clientscr/BOINCSaver_MacOS10_6_7.zip "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/Contents/MacOS"
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

cp -fpR "${BUILDPATH}/PostInstall.app" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app/Contents/Resources"

echo ${BRANDING_INFO} > "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app/Contents/Resources/PostInstall.app/Contents/Resources/Branding"
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

    # Code Sign the BOINC screensaver code for OS 10.6 and OS 10.7 if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/Contents/MacOS/BOINCSaver_MacOS10_6_7"

    # Code Sign the gfx_switcher utility embedded in BOINC screensaver if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/Contents/Resources/gfx_switcher"

    # Code Sign the gfx_cleanup utility embedded in BOINC screensaver if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/Contents/Resources/gfx_cleanup"

    # Code Sign the BOINC screensaver code for OS 10.8 and later if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Screen Savers/${SSAVERAPPNAME}.saver/"

    # Code Sign the BOINC client embedded in the Manager if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Applications/${MANAGERAPPNAME}.app/Contents/Resources/boinc"

    # Code Sign the BOINC Manager if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Applications/${MANAGERAPPNAME}.app"

    # Code Sign boinc_finish_install app embedded in the PostInstall app if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app/Contents/Resources/PostInstall.app/Contents/Resources/boinc_finish_install"

    # Code Sign the PostInstall app embedded in the BOINC installer app if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app/Contents/Resources/PostInstall.app"

    # Code Sign boinc_finish_install app embedded in BOINC uninstaller app if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app/Contents/Resources/boinc_finish_install"

    # Code Sign the BOINC uninstaller app if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/extras/${UNINSTALLERAPPNAME}.app"
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
cp -fpRL curl/ca-bundle.crt ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/

mkdir -p ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/switcher
cp -fpRL "${BUILDPATH}/switcher" ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/switcher/
cp -fpRL "${BUILDPATH}/setprojectgrp" ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/switcher/

sudo chown -R root:admin ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/*
sudo chmod -R u+rw-s,g+r-ws,o+r-w ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/*

cp -fpRL "${BUILDPATH}/SymbolTables/" ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_SymbolTables/

## If you wish to code sign the installer and uninstaller, create a file 
## ~/BOINCCodeSignIdentities.txt whose first line is the code signing identity
##
## Code signing using a registered Apple Developer ID is necessary for GateKeeper 
## with default settings to allow running downloaded applications under OS 10.8
if [ -n "${APPSIGNINGIDENTITY}" ]; then
    # Code Sign the BOINC installer application if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3/${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch/${INSTALLERAPPNAME}.app"

fi

cd ../BOINC_Installer/New_Release_${SHORTBRANDNAME}_$1_$2_$3

## Make everything in directory user-writable so project web code using auto-attach
## can delete it after inflating, modifying installer name and recompressing it.
sudo chmod -R u+w ./${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch

## Use ditto instead of zip utility to preserve resource forks and Finder attributes (custom icon, hide extension) 
ditto -ck --sequesterRsrc --keepParent ${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch ${SHORTBRANDNAME}_$1.$2.$3_macOSX_$arch.zip
ditto -ck --sequesterRsrc --keepParent ${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin ${SHORTBRANDNAME}_$1.$2.$3_$arch-apple-darwin.zip
ditto -ck --sequesterRsrc --keepParent ${SHORTBRANDNAME}_$1.$2.$3_macOSX_SymbolTables ${SHORTBRANDNAME}_$1.$2.$3_macOSX_SymbolTables.zip

#popd
cd "${BOINCPath}"

sudo rm -dfR ../BOINC_Installer/Installer\ Resources/
sudo rm -dfR ../BOINC_Installer/Installer\ Scripts/
sudo rm -dfR ../BOINC_Installer/Pkg_Root
sudo rm -dfR ../BOINC_Installer/locale
sudo rm -dfR ../BOINC_Installer/Installer\ templates
sudo rm -dfR ../BOINC_Installer/expandedVBox

return 0
