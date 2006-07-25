#! /bin/sh
# insecure.sh user group
#
# Undo making a BOINC installation secure.
# - Set file/dir ownership to the specified user and group
# - Remove BOINC groups and users
#
# Execute this as root in the BOINC directory

function remove_boinc_users() {
	userdel boinc_master
	userdel boinc_projects
	groupdel boinc_master
	groupdel boinc_projects
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

echo "Changing file ownership to user $user and group $group - OK? (y/n)"
read line
if [ "$line" != "y" ]
then
    exit
fi

chown -R ${user}:${group} .
chmod -R u+rw-s,g+r-w-s,o+r-w .
chmod 600 gui_rpc_auth.cfg
remove_boinc_users
