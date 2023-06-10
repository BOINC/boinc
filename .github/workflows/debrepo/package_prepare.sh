#!/bin/bash

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

OS="$1"      # distro for which the prepare is done
FULLPKG="$2" # full name of the package
PKG="$3"     # name of the artifact
BASEPKG="$4" # name of the artifact type

# validity check
case "$BASEPKG" in
  "linux_client-vcpkg")
     ;;
  "linux_manager-without-webview")
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

stat "${ROOT}/pkgs/$PKG.zip"
exit_on_fail

# unpack the github artifact
7z x "${ROOT}/pkgs/$PKG.zip"
exit_on_fail

stat "${BASEPKG}.7z"
exit_on_fail

# unpack the boinc archive
7z x "${BASEPKG}.7z"
exit_on_fail

rm -f "${BASEPKG}.7z"
exit_on_fail

find .

# required for deb package (same for debian and ubuntu)
mkdir -p DEBIAN
exit_on_fail

# specialized prepare 
case "$BASEPKG" in
  "linux_client-vcpkg")
     prepare_client
     ;;
  "linux_manager-without-webview")
     prepare_manager
     ;;
*)  echo "ERROR: Unknown package preparation requested"
    exit_usage
	;;
esac
popd

exit 0
