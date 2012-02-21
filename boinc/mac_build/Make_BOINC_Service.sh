#!/bin/sh

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2008 University of California
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
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#
#
##
# Script to set up Macintosh to run BOINC client as a daemon / service
# by Charlie Fenton 7/26/06 
# revised 1/6/08 to use launchd
##
## Note: this version of this script requires BOINC 5.10.34 or later 
## and OS 10.4 or later.  
##

## Usage:
##
## source Make_BOINC_Service.sh [path_to_client_dir] [path_to_data_dir]
##
## path_to_client_dir is needed only for stand-alone client or if Manager
##  is at non-standard location.
##
## path_to_data_dir is needed only for stand-alone client when data is in 
##  a different directory than the client.
##
## Directions for use:
##
## (1) Install BOINC Manager using the BOINC.pkg Installer Package, or 
##     install stand-alone client. 
## (2) Make sure you are logged in as a user with administrator 
##     privileges.
## (3) Run the Terminal application.
## (4) In the Terminal window, type "source" and a space.
## (5) Drag this script file from the Finder into the Terminal window.
## (6) If using the stand-alone client, or if the Manager is in a non-
##     standard location, drag the folder containing the client from 
##     the Finder into the Terminal window (or type the path excluding 
##     the trailing slash).
## (7) If using the stand-alone client, but the client is not in the data 
##     directory, drag the BOINC data folder from the Finder into the 
##     Terminal window (or type the path excluding the trailing slash).
## (8) Press the return key.
## (9) When prompted, enter your administrator password.
## (10) Restart the computer.
##
## The system will now start BOINC client as a daemon / service at 
## system startup.  You will still be able to control and monitor BOINC 
## using the BOINC Manager, but BOINC client will remain running after 
## you quit BOINC Manager, when the ScreenSaver quits, and even when 
## no user is logged in.
##
## Note: the BOINC ScreenSaver may not display graphics for some users 
## when BOINC Client is running as a daemon / service.  Thus will be 
## fixed with the release of BOINC 6.0 and project applications updated
## to use the BOINC 6 graphics API.
##

##
## When the system has launched BOINC Client as a daemon, you can stop the 
## BOINC Client from the Terminal by typing:
##   sudo launchctl stop edu.berkeley.boinc
## and you can restart the BOINC Client from the Terminal by typing:
##   sudo launchctl start edu.berkeley.boinc
##
## Note: The BOINC Client will quit immediately after launch if it detects 
## another instance of BOINC Client already running.  So these launchctl 
## commands will fail if the BOINC Client was started by the BOINC Manager 
## or ScreenSaver.
##
## If BOINC Client is not already running when the BOINC Manager is launched, 
## the Manager will start BOINC Client.  This will happen automatically at 
## login if you have BOINC Manager set as a login item for that user.  If 
## it was started by the Manager, then BOINC Client will quit when the BOINC 
## Manager quits.
##
## If BOINC Client is not already running when the BOINC ScreenSaver starts, 
## the ScreenSaver will start BOINC Client and will quit BOINC Client when 
## the ScreenSaver is dismissed.
##

## REMOVAL:
## To undo the effects of this script (i.e., to permanently stop running BOINC 
## as a daemon / service):
## (1) In the Finder, browse to the /Library/LaunchDaemons folder.
## (2) Drag the file edu.berkeley.boinc.plist to the trash.  (Finder will ask 
##     for your administrator user name and password).
## (3) Restart the computer.
##
## The system will no longer start BOINC client as a daemon / service 
## at system startup.   
##
## If you wish to completely remove BOINC from your computer, first 
## complete the above steps, then follow the directions in the web 
## page http://boinc.berkeley.edu/mac_advanced.php or in the README.rtf 
## file supplied with the BOINC installer. 
##

if [ $# -eq 0 ] ; then
    PATH_TO_CLIENT="/Applications/BOINCManager.app/Contents/Resources"
    PATH_TO_DATA="/Library/Application Support/BOINC Data"
    # Check for BOINC Manager with embedded BOINC Client
    if [ ! -f "${PATH_TO_CLIENT}/boinc" ]; then
        echo "  ***************************** ERROR ***************************"
        echo "  *                                                             *"
        echo "  *   Could not find BOINC Manager with embedded BOINC client   *"
        echo "  *   in Applications Directory.                                *"
        echo "  *                                                             *"
        echo "  *   Please install BOINC Manager before running this script.  *"
        echo "  *                                                             *"
        echo "  ***************************************************************"
        return 1
    fi
else
    PATH_TO_CLIENT="$1"
    PATH_TO_DATA="$1"
    # Check for stand-alone BOINC Client
    if [ ! -f "${PATH_TO_CLIENT}/boinc" ]; then
        echo "  ***************************** ERROR ***************************"
        echo "  *                                                             *"
        echo "  *   Could not find BOINC client at specified directory        *"
        echo "  *                                                             *"
        echo "  *   Please install BOINC client before running this script.   *"
        echo "  *                                                             *"
        echo "  ***************************************************************"
        return 1
    fi
fi

if [ $# -gt 1 ] ; then
    PATH_TO_DATA="$2"
fi

# Check for BOINC Data directory
if [ ! -d "${PATH_TO_DATA}" ]; then
    echo "  ****************** ERROR ******************"
    echo "  *                                         *"
    echo "  *   Could not find BOINC data directory   *"
    echo "  *                                         *"
    echo "  *******************************************"
    return 1
fi

# Delete the old-style boinc daemon StartupItem if present
sudo rm -fR /Library/StartupItems/boinc

# Delete old temporary working directory and files if present
rm -fR ~/boincStartupTemp
# Create new temporary working directory
mkdir -p ~/boincStartupTemp/

# Create file edu.berkeley.boinc.plist in temporary directory.
# (For some reason, we can't create the file directly in the final 
# destination directory, so we create it here and then move it.)
cat >> ~/boincStartupTemp/edu.berkeley.boinc.plist << ENDOFFILE
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>GroupName</key>
	<string>boinc_master</string>
	<key>Label</key>
	<string>edu.berkeley.boinc</string>
	<key>Program</key>
	<string>${PATH_TO_CLIENT}/boinc</string>
	<key>ProgramArguments</key>
	<array>
		<string>${PATH_TO_CLIENT}/boinc</string>
		<string>-redirectio</string>
		<string>-daemon</string>
	</array>
	<key>RunAtLoad</key>
	<true/>
	<key>UserName</key>
	<string>boinc_master</string>
	<key>WorkingDirectory</key>
	<string>${PATH_TO_DATA}/</string>
</dict>
</plist>
ENDOFFILE

sudo mv -f ~/boincStartupTemp/edu.berkeley.boinc.plist /Library/LaunchDaemons/

# Delete temporary working directory
rm -fR ~/boincStartupTemp

# Set ownership and permissions for our plist
sudo chown root:wheel /Library/LaunchDaemons/edu.berkeley.boinc.plist
sudo chmod 0444 /Library/LaunchDaemons/edu.berkeley.boinc.plist

return 0
