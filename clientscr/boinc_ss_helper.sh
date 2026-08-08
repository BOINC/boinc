# This file is part of BOINC.
# http:#boinc.berkeley.edu
# Copyright (C) 2020 University of California
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
# along with BOINC.  If not, see <http:#www.gnu.org/licenses/>.
#
#  boinc_ss_helper.sh
#
# Used under OS 10.15 Catalina and later to launch screensaver graphics apps
#
# BOINC screensaver plugin BOINCSaver.saver (BOINC Screensaver Coordinator)
# sends a run_graphics_app RPC to the BOINC client. The BOINC client then
# launches switcher, which submits this script to launchd as a LaunchAgent
# for the user that invoked the screensaver (the currently logged in user.)
# This script then launches gfx_switcher, which uses fork and execv to
# launch the project graphics app. gfx_switcher writes the graphics app's
# process ID to shared memory, to be read by the Screensaver Coordinator.
# gfx_switcher waits for the graphics app to exit and notifies then notifies
# the Screensaver Coordinator by writing 0 to the shared memory.
#
# We must go through launchd to establish a connection to the windowserver
# in the currently logged in user's space for use by the project graphics
# app. This script then launches gfx_switcher, which uses execv to launch
# the project graphics app.
# This Rube Goldberg process is necessary due to limitations on screensavers
# introduced in OS 10.15 Catalina.

#!/bin/sh

# argv[0] = path to this script
# argv[1] = directory
# argv[2] = branded screensaver name
# argv[3] = command for gfx_switcher: -default_gfx, -launch_gfx or -kill_gfx
# argv[4] = "boincscr", slot # or pid
# argv[5] = --fullscreen (not used for -kill_gfx)
# argv[6] = --ScreensaverLoginUser (not used for -kill_gfx)
# argv[7] = login user name (not used for -kill_gfx)

## For testing only:
## echo "number of args after argv[0] = $#" >> /Users/Shared/boinc_helper_script.txt
## echo "arg[1] = $1" >> /Users/Shared/boinc_helper_script.txt
## echo "arg[2] = $2" >> /Users/Shared/boinc_helper_script.txt
## echo "arg[3] = $3" >> /Users/Shared/boinc_helper_script.txt
## echo "arg[4] = $4" >> /Users/Shared/boinc_helper_script.txt
## if [ $# -eq 7 ]; then   ## bash does not count argv[0] in $#
## echo "arg[5] = $5" >> /Users/Shared/boinc_helper_script.txt
## echo "arg[6] = $6" >> /Users/Shared/boinc_helper_script.txt
## echo "arg[7] = $7" >> /Users/Shared/boinc_helper_script.txt
## fi

cd "$1"
pwd
if [ $# -eq 7 ]; then   ## bash does not count argv[0] in $#
"/Library/Screen Savers/$2.saver/Contents/Resources/gfx_switcher" $3 $4 $5 $6 $7
else
"/Library/Screen Savers/$2.saver/Contents/Resources/gfx_switcher" $3 $4
fi

# A new submit of edu.berkeley.boinc-ss_helper will be ignored if for some reason
# edu.berkeley.boinc-ss_helper is still loaded, so ensure it is removed.
launchctl remove edu.berkeley.boinc-ss_helper
