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

# Make a BOINC installation "secure" on a Macintosh with stand-alone BOINC Client
# The BOINC installer does this for a Macintosh installation with BOINC Manager; 
# the BOINC Manager contains the BOINC client embedded in its bundle.
# if you ran the BOINC installer, you do not need to use this script unless you 
# wish to run a separate copy of the stand-alone client in the BOINC Data 
# directory.
#
# Create groups and users, set file/dir ownership and protection
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
# sudo sh {path}/Mac_SA_Secure.sh
#
# Hint: you can enter the path to a directory or file by dragging its 
# icon from the Finder onto the Terminal window.
#
# You must have already put all necessary files into the boinc directory,
# including the boinc client, boinc_cmd application, ca-bundle.crt, plus 
# the switcher/ directory and its contents.
#
# This script also assumes that the user who runs it will be authorized 
# to administer BOINC. For convenience in administering BOINC, this script 
# adds the logged-in user to groups boinc_master and boinc_project 
# (i.e., adds these groups to the user's supplementary groups list.)
#
# In addition, you should add any other users who will administer BOINC 
# to groups boinc_master and boinc_project; e.g. for user mary:
# 
# sudo dscl . -merge /groups/boinc_master GroupMembership mary
# sudo dscl . -merge /groups/boinc_project GroupMembership mary
#
# To remove user mary from group boinc_master:
# sudo dscl . -delete /groups/boinc_master GroupMembership mary
# 

# Updated 1/28/10 for BOINC version 6.8.20, 6.10.30 and 6.11.1
# Updated 10/24/11 for OS 10.7.2 Lion
# Last updated 3/3/12 to create users and groups with IDs > 500 
# and to create RealName key with empty string as value (for users)
#
# WARNING: do not use this script with versions of BOINC older 
# than 6.8.20 and 6.10.30

function make_boinc_user() {
    DarwinVersion=`uname -r`;
    DarwinMajorVersion=`echo $DarwinVersion | sed 's/\([0-9]*\)[.].*/\1/' `;
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

    # Apple Developer Tech Support recommends using UID and GID greater 
    # than 500, but this causes problems on OS 10.4
    if [ "$DarwinMajorVersion" -gt 8 ]; then
        baseID="501"
    else
        baseID="25"
    fi



    # Check whether group already exists
    name=$(dscl . search /groups RecordName $1 | cut -f1 -s)
    if [ "$name" = "$1" ] ; then
        gid=$(dscl . read /groups/$1 PrimaryGroupID | cut -d" " -f2 -s)
    else
        # Find an unused group ID
        gid="$baseID"
        while true; do
            name=$(dscl . search /groups PrimaryGroupID $gid | cut -f1 -s)
            if [ -z "$name" ] ; then
                break
            fi
            gid=$[$gid +1]
        done
        dscl . -create /groups/$1
        dscl . -create /groups/$1 gid $gid
    fi
    
    # Check whether user already exists
    name=$(dscl . search /users RecordName $1 | cut -f1 -s)
    if [ -z "$name" ] ; then

        # Is uid=gid available?
        uid=$gid
        name=$(dscl . search /users UniqueID $uid | cut -f1 -s)
        if [ -n "$name" ] ; then
            # uid=gid already in use, so find an unused user ID
            uid="$baseID"
            while true; do
                name=$(dscl . search /users UniqueID $uid | cut -f1 -s)
                if [ -z "$name" ] ; then
                    break
                fi
                uid=$[$uid +1]
            done
        fi

        dscl . -create /users/$1
        dscl . -create /users/$1 uid $uid
        dscl . -create /users/$1 shell /usr/bin/false
        dscl . -create /users/$1 home /var/empty
        dscl . -create /users/$1 gid $gid
    fi
    
    
    ## Under OS 10.7 dscl won't directly create RealName key with empty 
    ## string as value but will allow changing value to empty string.
    ## -create replaces any previous value of the key if it already exists
    dscl . -create /users/boinc_master RealName $1
    dscl . -change /users/boinc_master RealName $1 ""
}

function make_boinc_users() {
    make_boinc_user boinc_master
    make_boinc_user boinc_project
}

function check_login() {
    if [ `whoami` != 'root' ]
    then
        echo 'This script must be run as root'
        exit
    fi
}

# set_perm path user group perm
#   set a file or directory to the given ownership/permissions
function set_perm() {
    chown $2:$3 "$1"
    chmod $4 "$1"
}

# same, but apply to all subdirs and files
#
function set_perm_recursive() {
    chown -R $2:$3 "$1"
    chmod -R $4 "$1"
}

# same, but apply to items in the given dir
#
function set_perm_dir() {
    for file in $(ls "$1")
    do
        path="$1/${file}"
        set_perm "${path}" $2 $3 $4
    done
}

function update_nested_dirs() {
   for file in $(ls "$1")
    do
	if [ -d "${1}/${file}" ] ; then
        chmod u+x,g+x,o+x "${1}/${file}"
		update_nested_dirs "${1}/${file}"
	fi
    done
}

check_login

echo "Changing directory $(pwd) file ownership to user and group boinc_master - OK? (y/n)"
read line
if [ "$line" != "y" ]
then
    exit
fi

if [ ! -x "switcher/switcher" ]
then
    echo "Can't find switcher application in directory $(pwd); exiting"
    exit
fi

make_boinc_users

# Check whether user is already a member of group boinc_master
#dscl . -read /Groups/boinc_master GroupMembership | grep -wq "$(LOGNAME)"
#if [ $? -ne 0 ]; then
dscl . -merge /groups/boinc_master GroupMembership "$(LOGNAME)"
#fi

# Check whether user is already a member of group boinc_project
#dscl . -read /Groups/boinc_project GroupMembership | grep -wq "$(LOGNAME)"
#if [ $? -ne 0 ]; then
dscl . -merge /groups/boinc_project GroupMembership "$(LOGNAME)"
#fi

# Set permissions of BOINC Data directory's contents:
#   ss_config.xml is world-readable so screensaver coordinator can read it
#   all other *.xml are not world-readable to keep authenticators private
#   gui_rpc_auth.cfg is not world-readable to keep RPC password private
#   all other files are world-readable so default screensaver can read them
set_perm_recursive . boinc_master boinc_master u+rw,g+rw,o+r-w
if [ -f gui_rpc_auth.cfg ] ; then
    set_perm gui_rpc_auth.cfg boinc_master boinc_master 0660
fi
chmod 0660 *.xml
if [ -f ss_config.xml ] ; then
    set_perm ss_config.xml boinc_master boinc_master 0661
fi

# Set permissions of BOINC Data directory itself
set_perm . boinc_master boinc_master 0771

if [ -d projects ] ; then
    set_perm_recursive projects boinc_master boinc_project u+rw,g+rw,o+r-w
    set_perm projects boinc_master boinc_project 0770
    update_nested_dirs projects
fi

if [ -d slots ] ; then
    set_perm_recursive slots boinc_master boinc_project u+rw,g+rw,o+r-w
    set_perm slots boinc_master boinc_project 0770
    update_nested_dirs slots
fi

# AppStats application must run setuid root (used in BOINC 5.7 through 5.8.14 only)
if [ -f switcher/AppStats ] ; then 
set_perm switcher/AppStats root boinc_master 4550
fi

set_perm switcher/switcher root boinc_master 04050
set_perm switcher/setprojectgrp boinc_master boinc_project 2500
set_perm switcher boinc_master boinc_master 0550

if [ -d locale ] ; then
    set_perm_recursive locale boinc_master boinc_master +X
    set_perm_recursive locale boinc_master boinc_master u+r-w,g+r-w,o+r-w
fi

if [ -f boinc ] ; then
    set_perm boinc boinc_master boinc_master 6555       # boinc client
fi

if [ -f boinccmd ] ; then
    set_perm boinccmd boinc_master boinc_master 0550
fi

if [ -f ss_config.xml ] ; then
    set_perm ss_config.xml boinc_master boinc_master 0664
fi

if [ -x /Applications/BOINCManager.app/Contents/MacOS/BOINCManager ] ; then 
    set_perm  /Applications/BOINCManager.app/Contents/MacOS/BOINCManager boinc_master boinc_master 0555
fi

if [ -x /Applications/BOINCManager.app/Contents/Resources/boinc ] ; then 
    set_perm /Applications/BOINCManager.app/Contents/Resources/boinc boinc_master boinc_master 6555
fi

# Version 6 screensaver has its own embedded switcher application, but older versions don't.
if [ -x "/Library/Screen Savers/BOINCSaver.saver/Contents/Resources/gfx_switcher" ] ; then 
    set_perm  "/Library/Screen Savers/BOINCSaver.saver/Contents/Resources/gfx_switcher" root boinc_master 4555
fi
