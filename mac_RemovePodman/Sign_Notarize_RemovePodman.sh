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
#  Sign_Notarize_RemovePodman.sh
#  
## You must have done the following before running this script:
##  * Created an app-specific password by following the instructions on
##      "Using app-specific passwords" at <https://support.apple.com/en-us/HT204397>.
##      NOTE: You cannot use your normal Apple ID password.
##  * Created a profile named "notarycredentials" in your keychain using the
##      "notarytool store-credentials" command (see "man notarytool" for details.)
##  * Created a file ~/BOINCCodeSignIdentities.txt whose first line is the code
## signing identity.

## Usage:
## cd to the directory containing RemovePodman.xcodeproj:
##     cd [path]/RemovePodman
##     source ./Sign_Notarize_RemovePodman.sh

#  Created 2/3/26.
#  
BOINCPath=$PWD

exec 7<"./Build_Deployment_Dir"
read -u 7 BUILDPATH

exec 8<"${HOME}/BOINCCodeSignIdentities.txt"
read APPSIGNINGIDENTITY <&8

echo
echo "***** Signing RemovePodman.app *****"
echo

sudo codesign -f -o runtime -s "${APPSIGNINGIDENTITY}" "${BUILDPATH}/RemovePodman.app"

echo
echo "***** Notarizing RemovePodman.app *****"
echo

ditto -ck --sequesterRsrc --keepParent "${BUILDPATH}/RemovePodman.app" "${BUILDPATH}/RemovePodman.zip"

xcrun notarytool submit "${BUILDPATH}/RemovePodman.zip" --keychain-profile "notarycredentials" --wait

mv "${BUILDPATH}/RemovePodman.app" "${BUILDPATH}/RemovePodman-orig.app"

open -W "${BUILDPATH}/RemovePodman.zip"

echo
echo "***** Stapling RemovePodman.app *****"
echo

xcrun stapler staple "${BUILDPATH}/RemovePodman.app"

mv "${BUILDPATH}/RemovePodman.zip" "${BUILDPATH}/RemovePodman-raw.zip"

echo
echo "***** Zipping notarized & stapled RemovePodman.app *****"
echo

ditto -ck --sequesterRsrc --keepParent "${BUILDPATH}/RemovePodman.app" "${BUILDPATH}/RemovePodman.zip"

return 0

