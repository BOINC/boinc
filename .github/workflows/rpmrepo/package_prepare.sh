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

ROOT=$(pwd)

FULLPKG="$1" # full name of the package
BASEPKG="$2" # name of the artifact type

RPM_BUILDROOT=$ROOT/rpmbuild/BUILD

function prepare_client() {
    # prepare dir structure
    mkdir -p $RPM_BUILDROOT/usr/local/bin
    exit_on_fail
    mkdir -p $RPM_BUILDROOT/etc/boinc-client $RPM_BUILDROOT/etc/default $RPM_BUILDROOT/etc/init.d
    exit_on_fail
    mkdir -p $RPM_BUILDROOT/usr/lib/systemd/system
    exit_on_fail
    mkdir -p $RPM_BUILDROOT/var/lib/boinc
    exit_on_fail
    mkdir -p $RPM_BUILDROOT/etc/bash_completion.d/ $RPM_BUILDROOT/etc/X11/Xsession.d
    exit_on_fail
    mkdir -p $RPM_BUILDROOT/usr/local/share/locale/
    exit_on_fail

    # copy files and directories
    cp $ROOT/pkgs/boinc $RPM_BUILDROOT/usr/local/bin/
    exit_on_fail
    cp $ROOT/pkgs/boinccmd $RPM_BUILDROOT/usr/local/bin/
    exit_on_fail
    cp $ROOT/pkgs/boinc-client.service $RPM_BUILDROOT/usr/lib/systemd/system/
    exit_on_fail
    cp $ROOT/pkgs/boinc-client.conf $RPM_BUILDROOT/etc/default/boinc-client
    exit_on_fail
    cp $ROOT/pkgs/boinc-client $RPM_BUILDROOT/etc/init.d/
    exit_on_fail
    cp $ROOT/pkgs/boinc.bash $RPM_BUILDROOT/etc/bash_completion.d/
    exit_on_fail
    cp $ROOT/pkgs/36x11-common_xhost-boinc $RPM_BUILDROOT/etc/X11/Xsession.d/
    exit_on_fail
    for dir in $(find $ROOT/pkgs/locale -maxdepth 1 -mindepth 1 -type d); do mkdir -p $RPM_BUILDROOT/usr/local/share/$dir/LC_MESSAGES; for file in $(find $dir -type f -iname BOINC-Client.mo); do cp $file $RPM_BUILDROOT/usr/local/share/$dir/LC_MESSAGES/; done; done
    exit_on_fail
    cp $ROOT/pkgs/all_projects_list.xml $RPM_BUILDROOT/var/lib/boinc/
    exit_on_fail
}

function prepare_manager() {
    # prepare dir structure
    mkdir -p $RPM_BUILDROOT/usr/local/bin
    exit_on_fail
    mkdir -p $RPM_BUILDROOT/usr/local/share/applications $RPM_BUILDROOT/usr/local/share/boinc-manager $RPM_BUILDROOT/usr/local/share/icons $RPM_BUILDROOT/usr/local/share/locale
    exit_on_fail

    # copy files and directories
    cp $ROOT/pkgs/boincmgr $RPM_BUILDROOT/usr/local/bin/
    exit_on_fail
    cp $ROOT/pkgs/boinc.desktop $RPM_BUILDROOT/usr/local/share/applications/
    exit_on_fail
    cp $ROOT/pkgs/boinc.png $RPM_BUILDROOT/usr/local/share/icons
    exit_on_fail
    cp $ROOT/pkgs/boinc.svg $RPM_BUILDROOT/usr/local/share/icons
    exit_on_fail
    cp -r $ROOT/pkgs/skins/ $RPM_BUILDROOT/usr/local/share/boinc-manager/
    exit_on_fail
    for dir in $(find $ROOT/pkgs/locale -maxdepth 1 -mindepth 1 -type d); do mkdir -p $RPM_BUILDROOT/usr/local/share/$dir/LC_MESSAGES; for file in $(find $dir -type f -iname BOINC-Manager.mo); do cp $file $RPM_BUILDROOT/usr/local/share/$dir/LC_MESSAGES/; done; done
    exit_on_fail
}

# setup RPM toplevel dirs
mkdir -p $ROOT/rpmbuild/{BUILD,BUILDROOT,RPMS,BUILDROOT,SOURCES,SPECS,SRPMS}
exit_on_fail

# setup of the archive
stat "$ROOT/$FULLPKG"
exit_on_fail

pushd "$ROOT/$FULLPKG"
exit_on_fail

mkdir -p $RPM_BUILDROOT
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
