#!/bin/csh

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
# Release Script for Macintosh BOINC Manager 10/31/07 by Charlie Fenton
## updated 11/18/09 by Charlie Fenton for OS 10.6 Snow Leopard
## updated 9/28/10 by Charlie Fenton for new BOINC skins
## updated 12/2/10 by Charlie Fenton to remove obsolete items
## updated 1/18/11 by Charlie Fenton to remove BOINC skins
## updated 11/9/11 by Charlie Fenton for XCode 4.1 and OS 10.7
## updated 11/26/11 by Charlie Fenton for new Default skin
## updated 11/27/11 by Charlie Fenton for new virtualbox directory
## updated 12/2/11 by Charlie Fenton to restore wrapper and reboot if needed
## updated 1/6/12 by Charlie Fenton to also install VirtualBox
## updated 6/22/12 by Charlie Fenton to code sign the installer and uninstaller
## updated 7/5/12 by Charlie Fenton to avoid using PackageMaker
## updated 7/31/12 by Charlie Fenton for Liberation font in boincscr
## updated 6/11/13 by Charlie Fenton for BOINC.mpkg, "BOINC + VirtualBox.mpkg"
## updated 6/18/13 by Charlie Fenton for localizable uninstaller
## updated 8/15/13 by Charlie Fenton to fix bug in localizable uninstaller
## updated 10/30/13 by Charlie Fenton to build a flat package
## updated 11/1/13 by Charlie Fenton to build installers both with and without VBox
## updated 11/18/13 by Charlie Fenton for Xcode 5.0.2
## updated 1/22/14 by Charlie Fenton: embed VBox uninstaller in BOINC uninstaller
## updated 9/30/14 by Charlie Fenton to code sign the BOINC client and Manager
## updated 12/16/14 by Charlie Fenton to name folders "x86_64" not "i686"
## updated 12/16/14 by Charlie Fenton to also code sign the installer package
## updated 12/17/14 by Charlie Fenton to fix typo in build of BOINC+VBox installer
## updated 4/7/15 by Charlie Fenton to comment on problem with BOINC+VBox installer
## updated 7/1/15 by Charlie Fenton for compatibility with OS 10.11
## updated 6/4/17 by Charlie Fenton for compatibility with Xcode versions > 5.0.2
## updated 10/19/17 by Charlie Fenton for different path to boinc_logo_black.jpg
## updated 11/11/17 by Charlie Fenton make all user-writable to help auto-attach
## updated 11/6/18 by Charlie Fenton to code sign for Apple "notarization"
## updated 11/4/19 by Charlie Fenton to code sign for new gfx_cleanup helper app
## updated 3/4/20 by Charlie Fenton to copy symbol tables directly from build
## Updated 7/29/20 by Charlie Fenton to build arm64 and x86_64 Universal2 Binary
## Updated 11/22/20 by Charlie Fenton to build DMG bare-core (apple-darwin) release
## Updated 11/26/20 by Charlie Fenton to let installer show message if MacOS too old
## Updated 5/27/21 to support zsh & detecting X86_64 features emulated by Rosetta 2
## Updated 6/24/21 allow installing BOINC on arm64 Macs without Rosetta 2 installed
## Updated 10/10/21 to eliminate ca-bundle.crt
## Updated 4/29/22 to eliminate obsolete clientscr/BOINCSaver_MacOS10_6_7.zip
## Updated 6/9/22 to eliminate harmless error message
## Updated 3/7/23 for boinc_finish_install to be a full application bundle
## Updated 4/30/23 code sign AddRemoveUser; eliminate old code signing workaround
## Updated 5/17/23 to add comments about notarize_BOINC.sh script
## Updated 2/12/25 to add support for Fix_BOINC_Users utility
## Updated 7/22/25 to add MacOS 26 support, no Finish_Install embedded in Postinstall.
## Updated 7/29/25 to add "BOINC podman" directory
## Updated 7/31/25 to add "Run_Podman" utility
## Updated 8/7/25 to add "gfx_ss_bridge" utility
## Updated 10/23/25 to put BOINCManager.app in "/Library/Application Support/"
##
## NOTE: This script requires Mac OS 10.7 or later, and uses XCode developer
##   tools.  So you must have installed XCode Developer Tools on the Mac
##   before running this script. You must code sign using OS 10.9 or later
##   for compatibility with Gatekeeper on OS 10.10 or later.
##
##

## NOTE: To build the executables under Lion and XCode 4, select from XCode's
## menu: "Product/Buildfor/Build for Archiving", NOT "Product/Archive"
## Under Mavericks and Xcode 5, select "Product/Build For/Profiling"

## To have this script build the combined BOINC+VirtualBox installer:
## * Create a directory named "VirtualBox Installer" in the same
##   directory which contains the root directory of the boinc tree.
## * Copy VirtualBox.pkg from the VirtualBox installer disk image (.dmg)
##   into this "VirtualBox Installer" directory.
## * Copy VirtualBox_Uninstall.tool from the VirtualBox installer disk
##   image (.dmg) into this "VirtualBox Installer" directory.
##
## NOTE: As of 5/7/15, I recommend against releasing the combined Macintosh
## BOINC+VirtualBox installer because each version of VirtualBox comes with
## its own uninsall script included as a separate command-line utility.
## Using the uninstall script from a previous version of VirtualBox may not
## work correctly, so we can't just add the current script to our "Uninstall
## BOINC" utility because the user may later upgrade to a newer version of
## VirtualBox.  We should not install VirtualBox as part of the BOINC install
## unless our BOINC uninstaller will uninstall it.  We can't expect BOINC users
## to be comfortable finding and running a command-line utility.
##
## We have asked Oracle to include their uninstall script inside the VirtualBox
## bundle, or some other standard place where our BOINC uninstaller can find
## the current version, but they have not yet done so.
##
## In addition, the notarize.sh script does not currently handle VirtualBox.

## Usage:
##
## If you wish to code sign the client, manager, installer and uninstaller,
## create a file ~/BOINCCodeSignIdentities.txt whose first line is the code
## signing identity.
## If you wish to also code sign the installer package, add a second line
## to ~/BOINCCodeSignIdentities.txt with the installer code signing identity.

##
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

## As of MacOS 10.15 Catalina, the OS does not allow the user to run downloaded
## software unless it has been "notarized" by Apple.
##
## To notarize the installer and uninstaller after successfully running this script,
## follow the instructions in the comments at the start of the script
##    mac_installer/notarize_boinc.sh.
##

if [ $# -lt 3 ]; then
echo "Usage:"
echo "   cd [path]/boinc"
echo "   source [path_to_this_script] major_version minor_version revision_number"
return 1
fi

#pushd ./
BOINCPath=$PWD

if [ "$4" = "-dev" ]; then
    exec 7<"mac_build/Build_Development_Dir"
    read -u 7 BUILDPATH
else
    exec 7<"mac_build/Build_Deployment_Dir"
    read -u 7 BUILDPATH
fi

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
cp -fp mac_installer/ReadMe.rtf ../BOINC_Installer/Installer\ Resources/

cp -fp mac_installer/complist.plist ../BOINC_Installer/Installer\ templates
cp -fp mac_installer/myDistribution ../BOINC_Installer/Installer\ templates

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

## Add a statement in the ReadMe telling Minimum required MacOS version, if known
OSVersion=`/usr/libexec/PlistBuddy -c "Print :LSMinimumSystemVersion" "${BUILDPATH}/BOINCManager.app/Contents/Info.plist"`
if [ $? -eq 0 ]; then
sed -i "" s/"<MINOSVERS>"/"^#NOTE: This version of BOINC requires MacOS <OSVERS> or later.^#"/g ../BOINC_Installer/Installer\ Resources/ReadMe.rtf
tr "^#" "\\\\\n" < ../BOINC_Installer/Installer\ Resources/ReadMe.rtf > /tmp/ReadMe.rtf
cp -f /tmp/ReadMe.rtf ../BOINC_Installer/Installer\ Resources/ReadMe.rtf
sed -i "" s/"<OSVERS>"/"$OSVersion"/g ../BOINC_Installer/Installer\ Resources/ReadMe.rtf

else
sed -i "" s/"<MINOSVERS>"/""/g ../BOINC_Installer/Installer\ Resources/ReadMe.rtf
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
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/projects/virtualbox

##cp -fpR "${BUILDPATH}/WaitPermissions.app" ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/

cp -fpRL "${BUILDPATH}/switcher" ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/switcher/
cp -fpRL "${BUILDPATH}/setprojectgrp" ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/switcher/
## cp -fpRL "${BUILDPATH}/AppStats" ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/switcher/

cd "${BOINCPath}/clientgui/skins"
cp -fpRL Default ../../../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/skins/
cd "${BOINCPath}"

cp -fp win_build/installerv2/redist/all_projects_list.xml ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/installer_projects_list.xml

cp -fp clientscr/res/boinc_logo_black.jpg ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/
cp -fp api/ttf/liberation-fonts-ttf-2.00.0/LiberationSans-Regular.ttf ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/LiberationSans-Regular.ttf
cp -fp clientscr/ss_config.xml ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/
cp -fpRL "${BUILDPATH}/boincscr" ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/
cp -fpRL "${BUILDPATH}/detect_rosetta_cpu" ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/
cp -fpRL "${BUILDPATH}/Fix_BOINC_Users" ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/
cp -fpRL "${BUILDPATH}/Run_Podman" ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/

cp -fpRL "${BUILDPATH}/BOINCManager.app" ../BOINC_Installer/Pkg_Root/Library/Application\ Support/

cp -fpRL "${BUILDPATH}/BOINCSaver.saver" ../BOINC_Installer/Pkg_Root/Library/Screen\ Savers/

echo "BrandId=0" > "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINCManager.app/Contents/Resources/Branding"
echo "BrandId=0" > ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/Branding

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

sudo rm -dfR ../BOINC_Installer/New_Release_$1_$2_$3/

mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/
mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch
mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras
mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin
mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_SymbolTables
mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-AddRemoveUser

cp -fp ../BOINC_Installer/Installer\ Resources/ReadMe.rtf ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch
sudo chown -R 501:admin ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/ReadMe.rtf
sudo chmod -R 644 ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/ReadMe.rtf

cp -fp COPYING ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/COPYING.txt
sudo chown -R 501:admin ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/COPYING.txt
sudo chmod -R 644 ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/COPYING.txt

cp -fp COPYING.LESSER ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/COPYING.LESSER.txt
sudo chown -R 501:admin ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/COPYING.LESSER.txt
sudo chmod -R 644 ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/COPYING.LESSER.txt

cp -fp COPYRIGHT ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/COPYRIGHT.txt
sudo chown -R 501:admin ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/COPYRIGHT.txt
sudo chmod -R 644 ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/COPYRIGHT.txt

cp -fpRL "${BUILDPATH}/Uninstall BOINC.app" ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras
# Copy the localization files for the uninstaller into its bundle
find locale -name 'BOINC-Setup.mo' | cut -d '/' -f 2 | awk '{print "\"../BOINC_Installer/locale/"$0"\""}' | xargs mkdir -p

find locale -name 'BOINC-Setup.mo' | cut -d '/' -f 2,3 | awk '{print "cp \"locale/"$0"\" \"../BOINC_Installer/locale/"$0"\""}' | bash

sudo cp -fpRL ../BOINC_Installer/locale "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app/Contents/Resources"

echo "BrandId=0" > "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app/Contents/Resources/Branding"

# Change BOINC_Finish_Install.app embedded in uninstaller to BOINC_Finish_Uninstall.app
sudo mv "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app/Contents/Resources/BOINC_Finish_Install.app" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app/Contents/Resources/BOINC_Finish_Uninstall.app"

# Change executable name of BOINC_Finish_Uninstall.app embedded in uninstaller to match app name
sudo mv "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app/Contents/Resources/BOINC_Finish_Uninstall.app/Contents/MacOS/BOINC_Finish_Install" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app/Contents/Resources/BOINC_Finish_Uninstall.app/Contents/MacOS/BOINC_Finish_Uninstall"

# Fix version number in Info.plist file of BOINC_Finish_Uninstall.app embedded
# in uninstaller
sudo plutil -replace CFBundleVersion -string `plutil -extract CFBundleVersion raw "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app/Contents/Info.plist"` "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app/Contents/Resources/BOINC_Finish_Uninstall.app/Contents/Info.plist"

sudo plutil -remove CFBundleShortVersionString "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app/Contents/Resources/BOINC_Finish_Uninstall.app/Contents/Info.plist"

# Fix bundle name in Info.plist file of BOINC_Finish_Uninstall.app embedded in uninstaller
sudo plutil -replace CFBundleName -string "BOINC_Finish_Uninstall" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app/Contents/Resources/BOINC_Finish_Uninstall.app/Contents/Info.plist"

sudo plutil -replace CFBundleExecutable -string "BOINC_Finish_Uninstall" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app/Contents/Resources/BOINC_Finish_Uninstall.app/Contents/Info.plist"

# If you re-enable this you must also change it in uninstall.cpp
# Change Bundle ID of BOINC_Finish_Uninstall.app
###sudo plutil -replace CFBundleIdentifier -string edu.berkeley.boinc.finish-uninstall "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app/Contents/Resources/BOINC_Finish_Uninstall.app/Contents/Info.plist"

# Replace icon reference in Info.plist file of BOINC_Finish_Uninstall.app embedded
# in uninstaller
sudo plutil -replace CFBundleIconFile -string "MacUninstaller.icns" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app/Contents/Resources/BOINC_Finish_Uninstall.app/Contents/Info.plist"

# Replace icon in Info.plist file of BOINC_Finish_Uninstall.app embedded in uninstaller
sudo rm -dfR "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app/Contents/Resources/BOINC_Finish_Uninstall.app/Contents/Resources/MacInstaller.icns"

sudo cp -fpRL "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app/Contents/Resources/MacUninstaller.icns" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app/Contents/Resources/BOINC_Finish_Uninstall.app/Contents/Resources"

sudo chown -R root:admin ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall\ BOINC.app
sudo chmod -R u+r-w,g+r-w,o+r-w ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall\ BOINC.app

# Copy the installer wrapper application "BOINC Installer.app"
cp -fpRL "${BUILDPATH}/BOINC Installer.app" ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/

## Allow the installer wrapper application to run on older versions of MacOS
## so it can display an appropriate error message.
/usr/libexec/PlistBuddy -c "Set :LSMinimumSystemVersion 10.0" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/BOINC Installer.app/Contents/Info.plist"

cp -fpR "${BUILDPATH}/PostInstall.app" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/BOINC Installer.app/Contents/Resources"

# Copy BOINC_Finish_Install.app into BOINC Data folder
cp -fpR "${BUILDPATH}/BOINC_Finish_Install.app" "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/"

echo "BrandId=0" > "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/BOINC Installer.app/Contents/Resources/PostInstall.app/Contents/Resources/Branding"

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

    # Code Sign the BOINC_Finish_Install.app in the BOINC Data folder
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/BOINC_Finish_Install.app"

    # Code Sign the gfx_switcher utility embedded in BOINC screensaver if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Screen Savers/BOINCSaver.saver/Contents/Resources/gfx_switcher"

    # Code Sign the gfx_cleanup utility embedded in BOINC screensaver if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Screen Savers/BOINCSaver.saver/Contents/Resources/gfx_cleanup"

    # Code Sign the gfx_ss_bridge utility embedded in BOINC screensaver if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Screen Savers/BOINCSaver.saver/Contents/Resources/gfx_ss_bridge"

    # Code Sign the BOINC screensaver code for OS 10.8 and later if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Screen Savers/BOINCSaver.saver"

    # Code Sign the BOINC client embedded in the Manager if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINCManager.app/Contents/Resources/boinc"

    # Code Sign the BOINC Manager if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/Pkg_Root/Library/Application Support/BOINCManager.app"

    # Code Sign the PostInstall app embedded in the BOINC installer app if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/BOINC Installer.app/Contents/Resources/PostInstall.app"

    # Code Sign BOINC_Finish_Uninstall app embedded in BOINC uninstaller app if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app/Contents/Resources/BOINC_Finish_Uninstall.app"

    # Code Sign the BOINC uninstaller app if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/extras/Uninstall BOINC.app"
fi

# Prepare to build the BOINC+VirtualBox installer if VirtualBox.pkg exists
VirtualBoxPackageName="VirtualBox.pkg"
if [ -f "../VirtualBox Installer/${VirtualBoxPackageName}" ]; then
    # Make a copy of the  BOINC installer app without the installer package
    sudo cp -fpRL "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_${arch}_vbox"

    # Copy the VirtualBox uninstall tool into the extras directory
    sudo cp -fpRL "../VirtualBox Installer/VirtualBox_Uninstall.tool" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_${arch}_vbox/extras/"

    sudo chown -R root:admin "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_${arch}_vbox/extras/VirtualBox_Uninstall.tool"
    sudo chmod -R u+r-w,g+r-w,o+r-w "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_${arch}_vbox/extras/VirtualBox_Uninstall.tool"

    # Copy the VirtualBox uninstall tool into the BOINC uninstaller
    sudo cp -fpRL "../VirtualBox Installer/VirtualBox_Uninstall.tool" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_${arch}_vbox/extras/Uninstall BOINC.app/Contents/Resources"

    sudo chown -R root:admin "../VirtualBox Installer/VirtualBox_Uninstall.tool" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_${arch}_vbox/extras/Uninstall BOINC.app/Contents/Resources/VirtualBox_Uninstall.tool"
    sudo chmod -R u+r-w,g+r-w,o+r-w "../VirtualBox Installer/VirtualBox_Uninstall.tool" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_${arch}_vbox/extras/Uninstall BOINC.app/Contents/Resources/VirtualBox_Uninstall.tool"
fi

# Build the installer package inside the wrapper application's bundle

cd "../BOINC_Installer/Installer templates"

pkgbuild --quiet --scripts "../Installer Scripts" --ownership recommended --identifier edu.berkeley.boinc --root "../Pkg_Root" --component-plist "./complist.plist" "./BOINC.pkg"

if [ -n "${INSTALLERSIGNINGIDENTITY}" ]; then
    productbuild --sign "${INSTALLERSIGNINGIDENTITY}" --quiet --resources "../Installer Resources/" --version "BOINC Manager $1.$2.$3" --distribution "./myDistribution" "../New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/BOINC Installer.app/Contents/Resources/BOINC.pkg"
else
    productbuild --quiet --resources "../Installer Resources/" --version "BOINC Manager $1.$2.$3" --distribution "./myDistribution" "../New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/BOINC Installer.app/Contents/Resources/BOINC.pkg"
fi
cd "${BOINCPath}"

# Build the BOINC+VirtualBox installer if VirtualBox.pkg exists
if [ -f "../VirtualBox Installer/${VirtualBoxPackageName}" ]; then
    cp -fpRL "mac_installer/V+BDistribution" "../BOINC_Installer/Installer templates/V+BDistribution"

    mkdir -p "../BOINC_Installer/expandedVBox/"

    pkgutil --expand "../VirtualBox Installer/${VirtualBoxPackageName}" "../BOINC_Installer/expandedVBox/VBox.pkg"

    pkgutil --flatten "../BOINC_Installer/expandedVBox/VBox.pkg/VBoxKEXTs.pkg" "../BOINC_Installer/Installer templates/VBoxKEXTs.pkg"

    pkgutil --flatten "../BOINC_Installer/expandedVBox/VBox.pkg/VBoxStartupItems.pkg" "../BOINC_Installer/Installer templates/VBoxStartupItems.pkg"

    pkgutil --flatten "../BOINC_Installer/expandedVBox/VBox.pkg/VirtualBox.pkg" "../BOINC_Installer/Installer templates/VirtualBox.pkg"

    pkgutil --flatten "../BOINC_Installer/expandedVBox/VBox.pkg/VirtualBoxCLI.pkg" "../BOINC_Installer/Installer templates/VirtualBoxCLI.pkg"

    cp -fpRL "../BOINC_Installer/expandedVBox/VBox.pkg/Resources/en.lproj" "../BOINC_Installer/Installer Resources"

    sudo rm -dfR "../BOINC_Installer/expandedVBox"

    cp -fp mac_installer/V+BDistribution "../BOINC_Installer/Installer templates"

    # Update version number
    sed -i "" s/"x.y.z"/"$1.$2.$3"/g "../BOINC_Installer/Installer templates/V+BDistribution"

    cd "../BOINC_Installer/Installer templates"

## TODO: Find a way to automatically set the VirtualBox version
    if [ -n "${INSTALLERSIGNINGIDENTITY}" ]; then
        productbuild --sign "${INSTALLERSIGNINGIDENTITY}" --quiet --resources "../Installer Resources" --version "BOINC Manager $1.$2.$3 + VirtualBox 4.3.12" --distribution "./V+BDistribution" "../New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_${arch}_vbox/BOINC Installer.app/Contents/Resources/BOINC.pkg"
    else
        productbuild --quiet --resources "../Installer Resources" --version "BOINC Manager $1.$2.$3 + VirtualBox 4.3.12" --distribution "./V+BDistribution" "../New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_${arch}_vbox/BOINC Installer.app/Contents/Resources/BOINC.pkg"
    fi

    cd "${BOINCPath}"
fi

# Build the stand-alone client distribution
cp -fpRL mac_build/Mac_SA_Insecure.sh ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/
cp -fpRL mac_build/Mac_SA_Secure.sh ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/
cp -fpRL "${BUILDPATH}/AddRemoveUser" ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-AddRemoveUser
cp -fp mac_installer/AddRemoveUser_ReadMe.rtf ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-AddRemoveUser/ReadMe.rtf
cp -fpRL COPYING ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/COPYING.txt
cp -fpRL COPYING.LESSER ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/COPYING.LESSER.txt
cp -fpRL COPYRIGHT ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/COPYRIGHT.txt
cp -fp mac_installer/License.rtf ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/
sudo chown -R 501:admin ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/*
sudo chmod -R 644 ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/*

mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir
cp -fpRL "${BUILDPATH}/boinc" ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/
cp -fpRL "${BUILDPATH}/boinccmd" ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/
cp -fpRL "${BUILDPATH}/detect_rosetta_cpu" ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/

mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/switcher
cp -fpRL "${BUILDPATH}/switcher" ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/switcher/
cp -fpRL "${BUILDPATH}/setprojectgrp" ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/switcher/

sudo chown -R root:admin ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/*
sudo chmod -R u+rw-s,g+r-ws,o+r-w ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/*

cp -fpRL "${BUILDPATH}/boinc.dSYM" ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_SymbolTables/
cp -fpRL "${BUILDPATH}/BOINCManager.app.dSYM" ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_SymbolTables/

## If you wish to code sign the installer and uninstaller, create a file
## ~/BOINCCodeSignIdentities.txt whose first line is the code signing identity
##
## Code signing using a registered Apple Developer ID is necessary for GateKeeper
## with default settings to allow running downloaded applications under OS 10.8
if [ -n "${APPSIGNINGIDENTITY}" ]; then
    # Code Sign the BOINC installer application if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_$arch/BOINC Installer.app"

    # Code Sign the stand-alone bare core boinc client if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/boinc"

    # Code Sign the AddRemoveUser app if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-AddRemoveUser/AddRemoveUser"

    # Code Sign detect_rosetta_cpu for the stand-alone boinc client if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/detect_rosetta_cpu"

    # Code Sign setprojectgrp for the stand-alone boinc client if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/switcher/setprojectgrp"

    # Code Sign switcher for the stand-alone boinc client if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/switcher/switcher"

    # Code Sign boinccmd for the stand-alone boinc client if we have a signing identity
    sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_$arch-apple-darwin/move_to_boinc_dir/boinccmd"
fi

cd ../BOINC_Installer/New_Release_$1_$2_$3

## Make everything in directory user-writable so project web code using auto-attach
## can delete it after inflating, modifying installer name and recompressing it.
sudo chmod -R u+w ./boinc_$1.$2.$3_macOSX_$arch

## Use ditto instead of zip utility to preserve resource forks and Finder attributes (custom icon, hide extension)
ditto -ck --sequesterRsrc --keepParent boinc_$1.$2.$3_macOSX_$arch boinc_$1.$2.$3_macOSX_$arch.zip
ditto -ck --sequesterRsrc --keepParent boinc_$1.$2.$3_macOSX_SymbolTables boinc_$1.$2.$3_macOSX_SymbolTables.zip
if [ -d boinc_$1.$2.$3_macOSX_${arch}_vbox ]; then
    ditto -ck --sequesterRsrc --keepParent boinc_$1.$2.$3_macOSX_${arch}_vbox boinc_$1.$2.$3_macOSX_${arch}_vbox.zip
fi
sudo hdiutil create -srcfolder boinc_$1.$2.$3_$arch-apple-darwin -ov -format UDZO boinc_$1.$2.$3_$arch-apple-darwin.dmg

## Command line tools such as AddRemoveuser cannot be notarized and so cannot be
## launched directly from the Finder (e.g. by double-clicking them), even if
## contained in a notarized dmg or zip file. But gatekeeper won't block them if
## they are run from within Terminal. For more information about this, see
## <https://developer.apple.com/forums/thread/127403>.
sudo hdiutil create -srcfolder boinc_$1.$2.$3_$arch-AddRemoveUser -ov -format UDZO boinc_$1.$2.$3_$arch-AddRemoveUser.dmg

#popd
cd "${BOINCPath}"

sudo rm -dfR ../BOINC_Installer/Installer\ Resources/
sudo rm -dfR ../BOINC_Installer/Installer\ Scripts/
sudo rm -dfR ../BOINC_Installer/Pkg_Root
sudo rm -dfR ../BOINC_Installer/locale
sudo rm -dfR ../BOINC_Installer/Installer\ templates
sudo rm -dfR ../BOINC_Installer/expandedVBox

return 0
