#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2023 University of California
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
    mkdir -p usr/bin
    exit_on_fail
    mkdir -p etc/boinc-client etc/default etc/init.d
    exit_on_fail
    mkdir -p usr/lib/systemd/system
    exit_on_fail
    mkdir -p var/lib/boinc
    exit_on_fail
    mkdir -p etc/bash_completion.d/
    exit_on_fail

    # copy files and directories
    mv boinc boinccmd usr/bin/
    exit_on_fail
    mv boinc-client.service usr/lib/systemd/system/
    exit_on_fail
    cp boinc-client etc/default/
    exit_on_fail
    mv boinc-client etc/init.d/
    exit_on_fail
    mv boinc-client.conf etc/boinc-client/boinc.conf
    exit_on_fail
    mv boinc.bash etc/bash_completion.d/
    exit_on_fail
}

function prepare_manager() {
    # prepare dir structure
    mkdir -p usr/bin
    exit_on_fail
    mkdir -p usr/share/applications usr/share/boinc-manager usr/share/icons/boinc usr/share/locale/boinc
    exit_on_fail

    # copy files and directories
    mv boincmgr usr/bin/
    exit_on_fail
    mv boinc.desktop usr/share/applications/
    exit_on_fail
    mv skins/ usr/share/boinc-manager/
    exit_on_fail
    mv locale/* usr/share/locale/boinc/
    exit_on_fail
    rm -rf locale/
}

ROOT=$(pwd)

FULLPKG="$1" # full name of the package
BASEPKG="$2" # name of the artifact type

# validity check
case "$BASEPKG" in
  "linux_client")
     ;;
  "linux_manager")
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

# required for deb package (same for debian and ubuntu)
mkdir -p DEBIAN
exit_on_fail

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
