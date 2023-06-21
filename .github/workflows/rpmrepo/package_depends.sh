#!/bin/bash

# support functions
function exit_usage() {
	printf "Usage: deb_depends.sh <os-version> <package-name>\n"
	exit 1
}

case "$1_$2" in
# fedora distros
"fc*_linux_client-vcpkg")
    echo "glibc,libXScrnSaver >= 1.2.3"
    ;;
# opensuse distros
"suse*_linux_client-vcpkg")
    echo "glibc,libXss1 >= 1.2.3"
    ;;

*)  echo "glibc"
	;;

esac

exit 0
