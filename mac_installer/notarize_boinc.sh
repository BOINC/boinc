#!/bin/sh

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2026 University of California
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
# Notarization Script for Macintosh BOINC Manager 5/17/23 by Charlie Fenton

# Updated 10/3/26 for Xcode27 support
##
## This script will notarize and staple the release created by the script
##    mac_installer/release_boinc.sh
##
## As of MacOS 10.15 Catalina, the OS does not allow the user to run downloaded
## software unless it has been "notarized" by Apple.
##
## cd to the root directory of the boinc tree, for example:
##     cd [path]/boinc
##
## Invoke this script with the three parts of the version number as arguments.
## For example, if the version is 3.2.1:
##     source [path_to_this_script] 3 2 1
##
## For testing only, you can use the development build by adding a fourth argument -dev
## For example, if the version is 3.2.1:
##     source [path_to_this_script] 3 2 1 -dev
##
## You must have done the following before running this script:
##  * Created an app-specific password by following the instructions on
##      "Using app-specific passwords" at <https://support.apple.com/en-us/HT204397>.
##      NOTE: You cannot use your normal Apple ID password.
##  * Created a profile named "notarycredentials" in your keychain using the
##      "notarytool store-credentials" command (see "man notarytool" for details.)
##  * Run the release_boinc.sh script
##
## - for more information:
##  $ xcrun notarytool --help
##  $ man stapler
##

Notarize_BOINCPath=$PWD

if [ "$4" = "-dev" ]; then
    exec 7<"mac_build/Build_Development_Dir"
else
    exec 7<"mac_build/Build_Deployment_Dir"
fi
read -u 7 Notarize_BUILDPATH

exec 7<&-   # Close fd 7

arch_to_notarize="x86_64"

Notarize_Products_Have_x86_64="no"
Notarize_Products_Have_arm64="no"
cd "${Notarize_BUILDPATH}"
lipo "BOINCManager.app/Contents/MacOS/BOINCManager" -verify_arch x86_64
if [ $? -eq 0 ]; then Notarize_Products_Have_x86_64="yes"; fi
lipo "BOINCManager.app/Contents/MacOS/BOINCManager" -verify_arch arm64
if [ $? -eq 0 ]; then Notarize_Products_Have_arm64="yes"; fi
if [ $Notarize_Products_Have_x86_64 = "no" ] && [ $Notarize_Products_Have_arm64 = "no" ]; then
    echo "ERROR: could not determine architecture of BOINC Manager"
    return 1
fi
if [ $Notarize_Products_Have_arm64 = "yes" ]; then
    if [ $Notarize_Products_Have_x86_64 = "yes" ]; then
        arch_to_notarize="universal"
    else
        arch_to_notarize="arm64"
    fi
fi

cd "${Notarize_BOINCPath}"

notarize_basepath="../BOINC_Installer/New_Release_$1_$2_$3"

echo
echo "***** Notarizing Installer and Uninstaller *****"
echo

xcrun notarytool submit "$notarize_basepath/boinc_$1.$2.$3_macOSX_$arch_to_notarize.zip" --keychain-profile "notarycredentials" --wait

if [ $? -ne 0 ]; then return 1; fi

##   *** STAPLING THE ORIGINAL FILES NEVER WORKS. FOR SOME REASON, WE MUST:
##       * REGENERATE THE DIRECTORY TREE FROM THE ZIP FILE WE SUBMITTED,
##       * STAPLE THE EXECUTABLES IN THE NEW TREE
##       * THEN CREATE A NEW ZIP FILE FROM THE STAPLED.
##   FOR SAFETY, I FIRST RENAME THE ORIGINAL DIRECTORY TREE AND ZIP FILE RATHER THAN
##   TRASHING THEM ***

mv "$notarize_basepath/boinc_$1.$2.$3_macOSX_$arch_to_notarize" "$notarize_basepath/boinc_$1.$2.$3_macOSX_$arch_to_notarize-orig"

open -W "$notarize_basepath/boinc_$1.$2.$3_macOSX_$arch_to_notarize.zip"

echo
echo "***** Stapling Installer *****"
echo

xcrun stapler staple "$notarize_basepath/boinc_$1.$2.$3_macOSX_$arch_to_notarize/BOINC Installer.app"

if [ $? -ne 0 ]; then return 1; fi

echo
echo "***** Stapling Uninstaller *****"
echo

xcrun stapler staple "$notarize_basepath/boinc_$1.$2.$3_macOSX_$arch_to_notarize/extras/Uninstall BOINC.app"

if [ $? -ne 0 ]; then return 1; fi

echo
echo "***** Zipping Installer and Uninstaller *****"
echo

mv "$notarize_basepath/boinc_$1.$2.$3_macOSX_$arch_to_notarize.zip" "$notarize_basepath/boinc_$1.$2.$3_macOSX_$arch_to_notarize-raw.zip"

ditto -ck --sequesterRsrc --keepParent "$notarize_basepath/boinc_$1.$2.$3_macOSX_$arch_to_notarize" "$notarize_basepath/boinc_$1.$2.$3_macOSX_$arch_to_notarize.zip"

##    *** Now notarize the command-line version ***
echo
echo "***** Notarizing command-line version *****"
echo

xcrun notarytool submit "$notarize_basepath/boinc_$1.$2.$3_$arch_to_notarize-apple-darwin.dmg" --keychain-profile "notarycredentials" --wait

if [ $? -ne 0 ]; then return 1; fi

##   *** STAPLING THE ORIGINAL DMG NEVER WORKS. FOR SOME REASON, WE MUST MAKE A
##        COPY OF THE ORIGINAL DMG AND STAPLE THAT. FOR SAFETY, I:
##       * FIRST RENAME THE ORIGINAL DMG RATHER THAN TRASHING IT
##       * MAKE A COPY WITH THE ORIGINAL NAME
##       * STAPLE THE NEW COPY (WITH THE ORIGINAL NAME) ***

mv "$notarize_basepath/boinc_$1.$2.$3_$arch_to_notarize-apple-darwin.dmg" "$notarize_basepath/boinc_$1.$2.$3_$arch_to_notarize-apple-darwin-raw.dmg"

cp "$notarize_basepath/boinc_$1.$2.$3_$arch_to_notarize-apple-darwin-raw.dmg" "$notarize_basepath/boinc_$1.$2.$3_$arch_to_notarize-apple-darwin.dmg"

echo
echo "***** Stapling command-line version *****"
echo

stapler staple "$notarize_basepath/boinc_$1.$2.$3_$arch_to_notarize-apple-darwin.dmg"

if [ $? -ne 0 ]; then return 1; fi

echo
echo "***** Notarizing AddRemoveUser *****"
echo

## Command line tools such as AddRemoveuser cannot be notarized and so cannot be
## launched directly from the Finder (e.g. by double-clicking them), even if
## contained in a notarized dmg or zip file. But gatekeeper won't block them if
## they are run from within Terminal, as long as the containing dmg or zip file
## has been notarized (the zip file can't be stapled, and doesn't need to be.)
## For more information about this, see
## <https://developer.apple.com/forums/thread/127403>.

xcrun notarytool submit "$notarize_basepath/boinc_$1.$2.$3_$arch_to_notarize-AddRemoveUser.dmg" --keychain-profile "notarycredentials" --wait

if [ $? -ne 0 ]; then return 1; fi

##   *** STAPLING THE ORIGINAL DMG NEVER WORKS. FOR SOME REASON, WE MUST MAKE A
##        COPY OF THE ORIGINAL DMG AND STAPLE THAT. FOR SAFETY, I:
##       * FIRST RENAME THE ORIGINAL DMG RATHER THAN TRASHING IT
##       * MAKE A COPY WITH THE ORIGINAL NAME
##       * STAPLE THE NEW COPY (WITH THE ORIGINAL NAME) ***

mv "$notarize_basepath/boinc_$1.$2.$3_$arch_to_notarize-AddRemoveUser.dmg" "$notarize_basepath/boinc_$1.$2.$3_$arch_to_notarize-AddRemoveUser-raw.dmg"

cp "$notarize_basepath/boinc_$1.$2.$3_$arch_to_notarize-AddRemoveUser-raw.dmg" "$notarize_basepath/boinc_$1.$2.$3_$arch_to_notarize-AddRemoveUser.dmg"

echo
echo "***** Stapling AddRemoveUser *****"
echo

stapler staple "$notarize_basepath/boinc_$1.$2.$3_$arch_to_notarize-AddRemoveUser.dmg"

if [ $? -ne 0 ]; then return 1; fi


return 0;
