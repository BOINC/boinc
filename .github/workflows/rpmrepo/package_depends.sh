#!/bin/bash

# See https://www.debian.org/doc/debian-policy/ch-relationships.html#s-binarydeps for the values
# accepted by the "Depends" field

# support functions
function exit_usage() {
	printf "Usage: deb_depends.sh <os-version> <package-name>\n"
	exit 1
}

case "$1_$2" in
# fedora distros
"38_linux_client-vcpkg")
    echo "glibc,libXScrnSaver >= 1.2.3"
    ;;
"37_linux_client-vcpkg")
    echo "glibc,libXScrnSaver >= 1.2.3"
    ;;

# opensuse distros
# "bullseye_linux_client-vcpkg")
#     echo "glibc,libXScrnSaver >= 1.2.3"
#     ;;
# "buster_linux_client-vcpkg")
#     echo "glibc,libXScrnSaver >= 1.2.3"
#     ;;

*)  echo "glibc"
	;;

esac

exit 0
