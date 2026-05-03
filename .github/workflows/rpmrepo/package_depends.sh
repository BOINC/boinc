#!/bin/bash

# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2026 University of California
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

case "$1" in
# fedora distros
"fc37" | "fc38" | "fc39" | "fc40" | "fc41" | "fc42" | "fc43" | "fc44")
    case "$2" in
    "client")
        echo "glibc,libXScrnSaver >= 1.2.3,ca-certificates,libatomic"
        ;;
    "manager")
        echo "glibc,libnotify,libX11,xkeyboard-config"
        ;;
    *)  echo "glibc"
        ;;
    esac
    ;;
# opensuse distros
"suse15_4" | "suse15_5" | "suse15_6" | "suse16_0")
    case "$2" in
    "client")
        echo "glibc,libXss1 >= 1.2.2,ca-certificates,libatomic1"
        ;;
    "manager")
        echo "glibc,libnotify4,libX11-6,xkeyboard-config"
        ;;
    *)  echo "glibc"
        ;;
    esac
    ;;
*)  echo "glibc"
	;;
esac

exit 0
