#!/bin/bash

# See https://www.debian.org/doc/debian-policy/ch-relationships.html#s-binarydeps for the values
# accepted by the "Depends" field

# support functions
function exit_usage() {
	printf "Usage: deb_depends.sh <os-version> <package-name>\n"
	exit 1
}

case "$1_$2" in
# ubuntu distros
"jammy_linux_client-vcpkg")
    echo "libc6,libxss1 (>= 1.2.3)"
    ;;
"focal_linux_client-vcpkg")
    echo "libc6,libxss1 (>= 1.2.3)"
    ;;

# debian distros
"bullseye_linux_client-vcpkg")
    echo "libc6,libxss1 (>= 1.2.3)"
    ;;
"buster_linux_client-vcpkg")
    echo "libc6,libxss1 (>= 1.2.3)"
    ;;

*)  echo "libc6"
	;;

esac

exit 0
