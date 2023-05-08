#! /bin/sh

# Berkeley Open Infrastructure for Network Computing
# http://boinc.berkeley.edu
# Copyright (C) 2006 University of California
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
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

# Make a BOINC installation "secure"
# Create groups and users, set file/dir ownership and protection
#
# Execute this as root in the BOINC directory
# You must have already run the installer script
# that creates the switcher/ and locale/ directories, and their contents

# In addition, you should add boinc_master and boinc_projects
# to the supplementary group list of users who will administer BOINC.
# e.g.:
# usermod -G boinc_master,boinc_projects -a mary

make_boinc_users() {
    groupadd boinc_master
    groupadd boinc_projects
    useradd boinc_master -g boinc_master
    useradd boinc_projects -g boinc_projects
}

check_login() {
    if [ `whoami` != 'root' ]
    then
        echo 'This script must be run as root'
        exit
    fi
}

# set_perm path user group perm
#   set a file or directory to the given ownership/permissions
set_perm() {
    chown $2:$3 "$1"
    chmod $4 "$1"
}

# same, but apply to all subdirs and files
#
set_perm_recursive() {
    chown -R $2:$3 "$1"
    chmod -R $4 "$1"
}

# same, but apply to items in the given dir
#
set_perm_dir() {
    for file in $(ls "$1")
    do
        path="$1/${file}"
        set_perm "${path}" $2 $3 $4
    done
}

update_nested_dirs() {
   chmod u+x,g+x,o+x "${1}"

   for file in $(ls "$1")
    do
	if [ -d "${1}/${file}" ] ; then
		update_nested_dirs "${1}/${file}"
	fi
    done
}

check_login

# If the user forgets to cd to the boinc data directory, this script can do serious damage
# so show the directory we are about to modify
echo "Changing directory $(pwd) file ownership to user and group boinc_master - OK? (y/n)"
read line
if [ "$line" != "y" ]
then
    exit
fi

# if the booinc client is not here, assume it is the wrong directory
if [ ! -f "boinc_client" ]
then
    echo "Can't find boinc_client in directory $(pwd); exiting"
    exit
fi

make_boinc_users

set_perm_recursive . boinc_master boinc_master u+rw,g+rw,o+r-w
set_perm . boinc_master boinc_master 0775
if [ -f gui_rpc_auth.cfg ] ; then
    set_perm gui_rpc_auth.cfg boinc_master boinc_master 0660
fi

if [ -d projects ] ; then
    set_perm_recursive projects boinc_master boinc_project u+rw,g+rw,o+r-w
    set_perm projects boinc_master boinc_master 0775
    update_nested_dirs projects
fi

if [ -d slots ] ; then
    set_perm_recursive slots boinc_master boinc_project u+rw,g+rw,o+r-w
    set_perm slots boinc_master boinc_master 0775
    update_nested_dirs slots
fi

set_perm switcher/switcher boinc_project boinc_project 6551
set_perm switcher/setprojectgrp boinc_master boinc_project 2500
set_perm switcher boinc_master boinc_master 0550

set_perm_recursive locale boinc_master boinc_master u+r-w,g+r-w,o-rwx

set_perm boinc_client boinc_master boinc_master 6555
set_perm boinc_manager boinc_master boinc_master 2555
