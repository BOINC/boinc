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
# 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# Mac_SA_Insecure.sh user group
#
# Undo making a Macintosh BOINC installation secure.  
# This script is for Macintosh installations with stand-alone BOINC CLient (i.e., no BOINC Manager.)
# - Set file/dir ownership to the specified user and group
# - Remove BOINC groups and users
#
# Execute this as root in the BOINC directory:
# cd {path_to_boinc_directory}
# sudo sh {path}/Mac_SA_Insecure.sh user group
#

function remove_boinc_users() {
sudo dscl . -delete /users/boinc_master
sudo dscl . -delete /groups/boinc_master

sudo dscl . -delete /users/boinc_project
sudo dscl . -delete /groups/boinc_project
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
chmod -R u+rw-s,g+r-w-s,o+r-w .
chmod 600 gui_rpc_auth.cfg
remove_boinc_users
