#!/bin/csh

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2023 University of California
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

basepath="../BOINC_Installer/New_Release_$1_$2_$3"

echo
echo "***** Notarizing Installer and Uninstaller *****"
echo

xcrun notarytool submit "$basepath/boinc_$1.$2.$3_macOSX_universal.zip" --keychain-profile "notarycredentials" --wait

if [ $? -ne 0 ]; then return 1; fi

##   *** STAPLING THE ORIGINAL FILES NEVER WORKS. FOR SOME REASON, WE MUST:
##       * REGENERATE THE DIRECTORY TREE FROM THE ZIP FILE WE SUBMITTED,
##       * STAPLE THE EXECUTABLES IN THE NEW TREE
##       * THEN CREATE A NEW ZIP FILE FROM THE STAPLED.
##   FOR SAFETY, I FIRST RENAME THE ORIGINAL DIRECTORY TREE AND ZIP FILE RATHER THAN
##   TRASHING THEM ***

mv "$basepath/boinc_$1.$2.$3_macOSX_universal" "$basepath/boinc_$1.$2.$3_macOSX_universal-orig"

open -W "$basepath/boinc_$1.$2.$3_macOSX_universal.zip"

echo
echo "***** Stapling Installer *****"
echo

xcrun stapler staple "$basepath/boinc_$1.$2.$3_macOSX_universal/BOINC Installer.app"

if [ $? -ne 0 ]; then return 1; fi

echo
echo "***** Stapling Uninstaller *****"
echo

xcrun stapler staple "$basepath/boinc_$1.$2.$3_macOSX_universal/extras/Uninstall BOINC.app"

if [ $? -ne 0 ]; then return 1; fi

echo
echo "***** Zipping Installer and Uninstaller *****"
echo

mv "$basepath/boinc_$1.$2.$3_macOSX_universal.zip" "$basepath/boinc_$1.$2.$3_macOSX_universal-raw.zip"

ditto -ck --sequesterRsrc --keepParent "$basepath/boinc_$1.$2.$3_macOSX_universal" "$basepath/boinc_$1.$2.$3_macOSX_universal.zip"

##    *** Now notarize the command-line version ***
echo
echo "***** Notarizing command-line version *****"
echo

xcrun notarytool submit "$basepath/boinc_$1.$2.$3_universal-apple-darwin.dmg" --keychain-profile "notarycredentials" --wait

if [ $? -ne 0 ]; then return 1; fi

##   *** STAPLING THE ORIGINAL DMG NEVER WORKS. FOR SOME REASON, WE MUST MAKE A
##        COPY OF THE ORIGINAL DMG AND STAPLE THAT. FOR SAFETY, I:
##       * FIRST RENAME THE ORIGINAL DMG RATHER THAN TRASHING IT
##       * MAKE A COPY WITH THE ORIGINAL NAME
##       * STAPLE THE NEW COPY (WITH THE ORIGINAL NAME) ***

mv "$basepath/boinc_$1.$2.$3_universal-apple-darwin.dmg" "$basepath/boinc_$1.$2.$3_universal-apple-darwin-raw.dmg"

cp "$basepath/boinc_$1.$2.$3_universal-apple-darwin-raw.dmg" "$basepath/boinc_$1.$2.$3_universal-apple-darwin.dmg"

echo
echo "***** Stapling command-line version *****"
echo

stapler staple "$basepath/boinc_$1.$2.$3_universal-apple-darwin.dmg"

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

xcrun notarytool submit "$basepath/boinc_$1.$2.$3_$arch-AddRemoveUser.dmg" --keychain-profile "notarycredentials" --wait

if [ $? -ne 0 ]; then return 1; fi

##   *** STAPLING THE ORIGINAL DMG NEVER WORKS. FOR SOME REASON, WE MUST MAKE A
##        COPY OF THE ORIGINAL DMG AND STAPLE THAT. FOR SAFETY, I:
##       * FIRST RENAME THE ORIGINAL DMG RATHER THAN TRASHING IT
##       * MAKE A COPY WITH THE ORIGINAL NAME
##       * STAPLE THE NEW COPY (WITH THE ORIGINAL NAME) ***

mv "$basepath/boinc_$1.$2.$3_$arch-AddRemoveUser.dmg" "$basepath/boinc_$1.$2.$3_$arch-AddRemoveUser-raw.dmg"

cp "$basepath/boinc_$1.$2.$3_$arch-AddRemoveUser-raw.dmg" "$basepath/boinc_$1.$2.$3_$arch-AddRemoveUser.dmg"

echo
echo "***** Stapling AddRemoveUser *****"
echo

stapler staple "$basepath/boinc_$1.$2.$3_$arch-AddRemoveUser.dmg"

if [ $? -ne 0 ]; then return 1; fi


return 0;
