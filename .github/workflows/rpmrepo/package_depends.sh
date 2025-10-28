#!/bin/bash

# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2025 University of California
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
function exit_usage() {
	printf "Usage: deb_depends.sh <os-version> <package-name>\n"
	exit 1
}

case "$1_$2" in
# fedora distros
"fc37_linux_client" | "fc38_linux_client" | "fc39_linux_client" | "fc40_linux_client" | "fc41_linux_client" | "fc42_linux_client")
    echo "glibc,libXScrnSaver >= 1.2.3,ca-certificates"
    ;;
# opensuse distros
"suse15_4_linux_client" | "suse15_5_linux_client" | "suse15_6_linux_client" | "suse16_0_linux_client")
    echo "glibc,libXss1 >= 1.2.2,ca-certificates"
    ;;

*)  echo "glibc"
	;;

esac

exit 0
