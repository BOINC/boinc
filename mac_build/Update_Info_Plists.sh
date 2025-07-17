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
#
#
# Update_Info_Plists.sh
# by Charlie Fenton 7/11/25
#

## Called from pre-actions in BOINC Xcode project when compiling BOINC.
## Note that Xcode runs build pre-actions pnly for the currently selected
## build target, not for any dependent targets that may also be built along
## with the selected target. So this script is called only once for any
## build operation, even the build_all target. The ovehead of running this
## script is minimal, so rather than comparing all the Info.plist file's
## modification dates to that of version.h, we just run this script for
## every build operation.
##
## Usage:
##    export WORKSPACE_PATH=
##    source {path}/mac_build/Update_Info_Plists.sh
## Called from pre-actions as:
##    source "$WORKSPACE_PATH/../../Update_Info_Plists.sh"
##
##
originalDir=`pwd`
directory=$(dirname "$WORKSPACE_PATH")
directory=$(dirname "$directory")
echo "Directory is " "$directory"
cd "$directory"
echo "About to check for " "$directory/build/Development/SetVersion"
echo "compared to " "$directory/../clientgui/mac/Setversion.cpp"
if [ ! -e "$directory/build/Development/SetVersion" ] || [ "$directory/../clientgui/mac/Setversion.cpp" -nt "$directory/build/Development/SetVersion" ]; then
    echo "About to run xcodebuild"
    xcodebuild -project boinc.xcodeproj -target SetVersion -configuration Development
    if [ $? -ne 0 ]; then
        echo "ERROR: xcodebuild failed"
        cd "${originalDir}"
        return 1
    fi
    echo "After xcodebuild, about to check for " "$directory/build/Development/SetVersion"
    if [ ! -e "$directory/build/Development/SetVersion" ]; then
        return 1
    fi
fi
###$directory="$directory/build/Development"
echo "Running SetVersion"
"$directory/build/Development/SetVersion"
cd "${originalDir}"
return 0
