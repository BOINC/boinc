#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2024 University of California
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

# support functions
function exit_on_fail() {
	errcode=$?
	if [[ ! "$errcode" -eq "0" ]]; then
		printf "Failed command, exitcode: %d\n" "$errcode"
		exit 1
	fi
}

function exit_usage() {
	printf "Usage: prepare_package.sh <os-version> <full-name> <package-name> <root-package>\n"
	exit 1
}

function prepare_client() {
    # prepare dir structure
    mkdir -p usr/local/bin
    exit_on_fail
    mkdir -p etc/boinc-client etc/default etc/init.d
    exit_on_fail
    mkdir -p usr/lib/systemd/system
    exit_on_fail
    mkdir -p var/lib/boinc
    exit_on_fail
    mkdir -p etc/bash_completion.d/ etc/X11/Xsession.d
    exit_on_fail
    mkdir -p usr/local/share/locale/
    exit_on_fail
    mkdir -p DEBIAN
    exit_on_fail

    # copy files and directories
    mv postinst DEBIAN/
    exit_on_fail
    mv boinc boinccmd usr/local/bin/
    exit_on_fail
    mv boinc-client.service usr/lib/systemd/system/
    exit_on_fail
    mv boinc-client.conf etc/default/boinc-client
    exit_on_fail
    mv boinc-client etc/init.d/
    exit_on_fail
    mv boinc.bash etc/bash_completion.d/
    exit_on_fail
    mv 36x11-common_xhost-boinc etc/X11/Xsession.d/
    exit_on_fail
    for dir in $(find ./locale -maxdepth 1 -mindepth 1 -type d); do mkdir -p usr/local/share/$dir/LC_MESSAGES; for file in $(find $dir -type f -iname BOINC-Client.mo); do mv $file usr/local/share/$dir/LC_MESSAGES/; done; done
    exit_on_fail
    rm -rf locale/
    mv all_projects_list.xml var/lib/boinc/
    exit_on_fail
}

function prepare_manager() {
    # prepare dir structure
    mkdir -p usr/local/bin
    exit_on_fail
    mkdir -p usr/local/share/applications usr/local/share/boinc-manager usr/local/share/icons usr/local/share/locale/
    exit_on_fail
    mkdir -p DEBIAN
    exit_on_fail

    # copy files and directories
    mv boincmgr usr/local/bin/
    exit_on_fail
    mv boinc.desktop usr/local/share/applications/
    exit_on_fail
    mv boinc.png usr/local/share/icons/
    exit_on_fail
    mv boinc.svg usr/local/share/icons/
    exit_on_fail
    mv skins/ usr/local/share/boinc-manager/
    exit_on_fail
    for dir in $(find ./locale -maxdepth 1 -mindepth 1 -type d); do mkdir -p usr/local/share/$dir/LC_MESSAGES; for file in $(find $dir -type f -iname BOINC-Manager.mo); do mv $file usr/local/share/$dir/LC_MESSAGES/; done; done
    exit_on_fail
    rm -rf locale/
    rm -rf res/
}

ROOT=$(pwd)

FULLPKG="$1" # full name of the package
BASEPKG="$2" # name of the artifact type

# validity check
case "$BASEPKG" in
  "linux_client" | "linux_manager")
     ;;

*)  echo "ERROR: Unknown package preparation requested"
    exit_usage
	;;
esac

# setup of the archive
stat "$ROOT/$FULLPKG"
exit_on_fail

pushd "$ROOT/$FULLPKG"
exit_on_fail

stat "${ROOT}/pkgs/${BASEPKG}.7z"
exit_on_fail

# unpack the boinc archive
7z x "${ROOT}/pkgs/${BASEPKG}.7z"
exit_on_fail

find .

# specialized prepare
case "$BASEPKG" in
  "linux_client")
     prepare_client
     ;;
  "linux_manager")
     prepare_manager
     ;;
*)  echo "ERROR: Unknown package preparation requested"
    exit_usage
	;;
esac
popd

exit 0
