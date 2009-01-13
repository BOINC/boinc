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

# insecure.sh user group
#
# Undo making a BOINC installation secure.
# - Set file/dir ownership to the specified user and group
# - Remove BOINC groups and users
#
# Execute this as root in the BOINC directory

remove_boinc_users() {
	userdel boinc_master
	userdel boinc_projects
	groupdel boinc_master
	groupdel boinc_projects
}

check_login() {
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

# If the user forgets to cd to the boinc data directory,
# this script can do serious damage
# so show the directory we are about to modify
echo "Changing directory $(pwd) ownership to user $user and group $group - OK? (y/n)"
read line
if [ "$line" != "y" ]
then
    exit
fi

# if the boinc client is not here, assume it is the wrong directory
if [ ! -f "boinc_client" ]
then
    echo "Can't find boinc_client in directory $(pwd); exiting"
    exit
fi

chown -R ${user}:${group} .
chmod -R u+rw-s,g+r-w-s,o+r-w .
chmod 600 gui_rpc_auth.cfg
remove_boinc_users
