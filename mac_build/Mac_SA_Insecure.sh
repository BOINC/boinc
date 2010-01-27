#! /bin/sh

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

# Mac_SA_Insecure.sh user group
#
# Undo making a Macintosh BOINC installation secure.  
# - Set file/dir ownership to the specified user and group
# - Remove BOINC groups and users
#
# IMPORTANT NOTE: earlier versions of the Mac_SA_Insecure.sh and 
# Mac_SA_Secure.sh scripts had serious problems when run under OS 10.3.x.
# They sometimes created bad users and groups with IDs that were duplicates 
# of other users and groups.  They ran correctly under OS 10.4.x
#
# If you ran an older version of either script under OS 10.3.x, you should 
# first run the current version of Mac_SA_Insecure.sh to delete the bad 
# entries and then run Mac_SA_Secure.sh to create new good entries.
#
#
# Execute this as root in the BOINC directory:
# cd {path_to_boinc_directory}
# sudo sh {path}/Mac_SA_Insecure.sh user group
#
# After running this script, the boinc client must be run with 
# the --insecure option.
# NOTE: running BOINC with security disabled is not recommended.
#
# Last updated 1/26/10 for BOINC versions 6.8.19, 6.10.30 and 6.11.1 
# WARNING: do not use this script with older versions of BOINC older 
# than 6.8.17 and 6.10.3

function remove_boinc_users() {
    name=$(dscl . search /users RecordName boinc_master | cut -f1 -s)
    if [ "$name" = "boinc_master" ] ; then
        sudo dscl . -delete /users/boinc_master
    fi

    name=$(dscl . search /groups RecordName boinc_master | cut -f1 -s)
    if [ "$name" = "boinc_master" ] ; then
        sudo dscl . -delete /groups/boinc_master
    fi
    
    name=$(dscl . search /users RecordName boinc_project | cut -f1 -s)
    if [ "$name" = "boinc_project" ] ; then
        sudo dscl . -delete /users/boinc_project
    fi

    name=$(dscl . search /groups RecordName boinc_project | cut -f1 -s)
    if [ "$name" = "boinc_project" ] ; then
        sudo dscl . -delete /groups/boinc_project
    fi
}

function check_login() {
    if [ `whoami` != 'root' ]
    then
        echo 'This script must be run as root'
        exit
    fi
}

check_login

if [ $# -eq 2 ]
then
    user=$1
    group=$2
else
    echo "usage: $0 user group"
    exit
fi

echo "Changing directory $(pwd) file ownership to user $user and group $group - OK? (y/n)"
read line
if [ "$line" != "y" ]
then
    exit
fi

if [ ! -f "boinc" ]
then
    echo "Can't find boinc Client in directory $(pwd); exiting"
    exit
fi

chown -R ${user}:${group} .
chmod -R +Xu+rw-s,g+r-w-s,o+r-w .
chmod 600 gui_rpc_auth.cfg

if [ -f switcher/AppStats ] ; then 
# AppStats application must run setuid root (used in BOINC 5.7 through 5.8.14 only)
chown root:${group} switcher/AppStats
chmod 4550 switcher/AppStats
fi

if [ -x /Applications/BOINCManager.app/Contents/MacOS/BOINCManager ] ; then 
    chown ${user}:${group} /Applications/BOINCManager.app/Contents/MacOS/BOINCManager
    chmod -R u+r-ws,g+r-ws,o+r-ws /Applications/BOINCManager.app/Contents/MacOS/BOINCManager
fi

if [ -x /Applications/BOINCManager.app/Contents/Resources/boinc ] ; then 
    chown ${user}:${group} /Applications/BOINCManager.app/Contents/Resources/boinc
    chmod -R u+r-ws,g+r-ws,o+r-ws /Applications/BOINCManager.app/Contents/Resources/boinc
fi

# Version 6 screensaver has its own embedded switcher application, but older versions don't.
if [ -x "/Library/Screen Savers/BOINCSaver.saver/Contents/Resources/gfx_switcher" ] ; then 
    chown ${user}:${group} "/Library/Screen Savers/BOINCSaver.saver/Contents/Resources/gfx_switcher"
    chmod -R u+r-ws,g+r-ws,o+r-ws "/Library/Screen Savers/BOINCSaver.saver/Contents/Resources/gfx_switcher"
fi

remove_boinc_users
