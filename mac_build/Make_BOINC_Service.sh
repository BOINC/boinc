#!/bin/sh

# Berkeley Open Infrastructure for Network Computing
# http://boinc.berkeley.edu
# Copyright (C) 2005 University of California
#
# This is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation;
# either version 2.1 of the License, or (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# To view the GNU Lesser General Public License visit
# http://www.gnu.org/copyleft/lesser.html
# or write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#
##
# Script to set up Macintosh to run BOINC client as a daemon / service
# by Charlie Fenton 4/7/06
##

## Usage:
## (1) Install BOINC Manager using the BOINC.pkg Installer Package. 
## (2) Make sure you are logged in as a user with administrator 
##     privileges.
## (3) Run the Terminal application.
## (4) In the Terminal window, type "source" and a space.
## (5) Drag this script file from the Finder into the Terminal window.
## (6) Press the return key.
## (7) When prompted, enter your administrator password.
## (8) Restart the computer.
##
## The system will now start BOINC client as a daemon / service at 
## system startup.  You will still be able to control and monitor BOINC 
## using the BOINC Manager, but BOINC client will remain running after 
## you quit BOINC Manager, when the ScreenSaver quits, and even when 
## no user is logged in.
##
## Note: the BOINC ScreenSaver may not display graphics for some users 
## when BOINC Client is running as a daemon / service.  We hope to fix 
## this in the near future.
##

## Removal (to stop running BOINC as a daemon / service):
## (1) In the Finder, browse to the /Library/StartupItems folder.
## (2) Drag the boinc directory to the trash.  (Finder will ask for 
##     your administrator user name and password).
## (3) Restart the computer.
##
## The system will no longer start BOINC client as a daemon / service 
## at system startup.  
##
## Running BOINC Manager will star BOINC Client.  This will happen 
## automatically at login if you have BOINC Manager set as a login item 
## for that user.  BOINC Client will quit when the BOINC Manager quits.
##
## If BOINC Client is not already running when the BOINC ScreenSaver 
## starts, the ScreenSaver will start BOINC Client and will quit 
## BOINC Client when the ScreenSaver is dismissed.
##
## If you wish to completely remove BOINC from your computer, first 
## complete the above steps, then follow the directions in the web 
## page http://boinc.berkeley.edu/mac_advanced.php or in the README.rtf 
## file supplied with the BOINC installer. 
##

# Check for BOINC Manager with embedded BOINC Client
if [ ! -f /Applications/BOINCManager.app/Contents/Resources/boinc ]; then
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

# Create /Library/StartupItems/boinc/ directory if necessary
if [ ! -d /Library/StartupItems/boinc ]; then
    sudo mkdir -p /Library/StartupItems/boinc
fi

# Delete old temporary working directory and files if present
rm -f ~/boincStartupTemp/
# Create new temporary working directory
mkdir -p ~/boincStartupTemp/

# Create the shell script to start BOINC client in temporary directory
# (For some reason, we can't create the files directly in the final 
# destination directory, so we create them here and them move them.)
cat >> ~/boincStartupTemp/boinc << ENDOFFILE
#!/bin/sh

##
# Start BOINC client as a daemon
##

. /etc/rc.common

StartService ()
{
        if [ -d "/Library/Application Support/BOINC Data" ]; then 
            echo "Starting BOINC"
            /Applications/BOINCManager.app/Contents/Resources/boinc -redirectio -dir "/Library/Application Support/BOINC Data/" &
        fi
}

StopService ()
{
    return 0
}

RestartService ()
{
    return 0
}

RunService "\$1"

ENDOFFILE


# Create the BOINC StartupParameters.plist file in temporary directory
cat >> ~/boincStartupTemp/StartupParameters.plist << ENDOFFILE
{
  Description     = "BOINC client daemon";
  Provides        = ("BOINC client daemon");
  Requires        = ("NFS");
  OrderPreference = "Last";
}
ENDOFFILE

sudo mv -f ~/boincStartupTemp/boinc /Library/StartupItems/boinc/boinc

sudo mv -f ~/boincStartupTemp/StartupParameters.plist /Library/StartupItems/boinc/StartupParameters.plist

# Delete temporary working directory
rm -fR ~/boincStartupTemp/

# Set ownership and permissions as needed
sudo chmod +x /Library/StartupItems/boinc/boinc
sudo chown -R root:wheel /Library/StartupItems/boinc

return 0
