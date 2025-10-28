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

case "$1_$2" in
"fc37_linux_client" | "fc38_linux_client" | "fc39_linux_client" | "fc40_linux_client" | "fc41_linux_client" | "fc42_linux_client" | "suse15_4_linux_client" | "suse15_5_linux_client" | "suse15_6_linux_client" | "suse16_0_linux_client")
    echo """/etc/default/*
/etc/init.d/*
/etc/bash_completion.d/*
/etc/X11/Xsession.d/*
/var/lib/*
/usr/lib/systemd/system/*
/usr/local/bin/*
/usr/lib/*
/usr/local/share/locale/*
"""
    ;;

"fc37_linux_manager" | "fc38_linux_manager" | "fc39_linux_manager" | "fc40_linux_manager" | "fc41_linux_manager" | "fc42_linux_manager" | "suse15_4_linux_manager" | "suse15_5_linux_manager" | "suse15_6_linux_manager" | "suse16_0_linux_manager")
    echo """/usr/local/bin/*
/usr/local/share/applications/*
/usr/local/share/boinc-manager/*
/usr/local/share/locale/*
/usr/local/share/icons/*
"""
    ;;

*)  echo "failed"
	;;

esac

exit 0
