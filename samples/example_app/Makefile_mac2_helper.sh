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
#
#
# Script to build Macintosh example_app using Makefile
#
# by Charlie Fenton 10/2/26

# Called from Makefile_mac2

SDKPATH=`xcodebuild -version -sdk macosx Path`
result=$?

## Set deployment target to oldest MacOS version supported by this Xcode version
if [ $result -eq 0 ]; then
    targetOSVers=`plutil -extract SupportedTargets.macosx.MinimumDeploymentTarget raw -n "${SDKPATH}/SDKSettings.plist"`
    result=$?
fi
if [ $result -ne 0 ]; then
    echo "Failed to set deployment target MacOS version number"
    exit $result
fi
## Convert MacOS version number to form used by Availability Macros
IFS='.' read -r MAJOR MINOR PATCH <<< "$targetOSVers"
MINOR=${MINOR:-0}
PATCH=${patch:-0}
if [ "$MAJOR" -eq 10 ]; then
    # Legacy macOS 10.x format: 10xx00
    # Uses printf to pad the minor version to 2 digits
    printf -v MAC_OS_VERSION "10%02d00" "$MINOR"
else
    # macOS 11.0+ format: xx0000
    printf -v MAC_OS_VERSION "%02d0000" "$MAJOR"
fi

# Output the variables formatted as Makefile syntax
echo $targetOSVers $MAC_OS_VERSION

exit 0
