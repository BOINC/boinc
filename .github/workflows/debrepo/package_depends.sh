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

# See https://www.debian.org/doc/debian-policy/ch-relationships.html#s-binarydeps for the values
# accepted by the "Depends" field

# support functions
function exit_usage() {
	printf "Usage: deb_depends.sh <os-version> <package-name>\n"
	exit 1
}

case "$1_$2" in
# ubuntu distros
"focal_linux_client")
    echo "libc6,libxss1 (>= 1.2.3)"
    ;;
"jammy_linux_client")
    echo "libc6,libxss1 (>= 1.2.3)"
    ;;

# debian distros
"buster_linux_client")
    echo "libc6,libxss1 (>= 1.2.3)"
    ;;
"bullseye_linux_client")
    echo "libc6,libxss1 (>= 1.2.3)"
    ;;
"bookworm_linux_client")
    echo "libc6,libxss1 (>= 1.2.3)"
    ;;

*)  echo "libc6"
	;;

esac

exit 0
