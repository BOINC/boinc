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

# See https://www.debian.org/doc/debian-policy/ch-relationships.html#s-binarydeps for the values
# accepted by the "Depends" field

# support functions
function exit_usage() {
	printf "Usage: deb_depends.sh <os-version> <package-name>\n"
	exit 1
}

case "$1_$2" in
"focal_linux_client" | "jammy_linux_client" | "noble_linux_client" | "buster_linux_client" | "bullseye_linux_client" | "bookworm_linux_client" | "trixie_linux_client")
    echo "libc6,libxss1 (>= 1.2.3),ca-certificates"
    ;;
*)  echo "libc6"
	;;

esac

exit 0
