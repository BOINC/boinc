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
    mkdir -p SOURCES/usr/bin
    exit_on_fail
    mkdir -p SOURCES/etc/boinc-client SOURCES/etc/default SOURCES/etc/init.d
    exit_on_fail
    mkdir -p SOURCES/usr/lib/systemd/system
    exit_on_fail
    mkdir -p SOURCES/var/lib/boinc
    exit_on_fail
    mkdir -p SOURCES/etc/bash_completion.d/
    exit_on_fail

    # copy files and directories
    mv boinc boinccmd SOURCES/usr/bin/
    exit_on_fail
    mv boinc-client.service SOURCES/usr/lib/systemd/system/
    exit_on_fail
    cp boinc-client SOURCES/etc/default/
    exit_on_fail
    mv boinc-client SOURCES/etc/init.d/
    exit_on_fail
    mv boinc-client.conf SOURCES/etc/boinc-client/boinc.conf
    exit_on_fail
    mv boinc.bash SOURCES/etc/bash_completion.d/
    exit_on_fail
}

function prepare_manager() {
    # prepare dir structure
    mkdir -p SOURCES/usr/bin
    exit_on_fail
    mkdir -p SOURCES/usr/share/applications SOURCES/usr/share/boinc-manager SOURCES/usr/share/icons/boinc SOURCES/usr/share/locale/boinc
    exit_on_fail

    # copy files and directories
    mv boincmgr SOURCES/usr/bin/
    exit_on_fail
    mv boinc.desktop SOURCES/usr/share/applications/
    exit_on_fail
    mv skins/ SOURCES/usr/share/boinc-manager/
    exit_on_fail
    mv locale/* SOURCES/usr/share/locale/boinc/
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

# setup RPM toplevel dirs
mkdir -p rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
exit_on_fail

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
