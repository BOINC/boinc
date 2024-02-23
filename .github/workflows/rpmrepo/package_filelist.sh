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

case "$1_$2" in
# fedora variants
"fc*_linux_client" | "suse*_linux_client")
    echo """/etc/default/*
/etc/init.d/*
/etc/bash_completion.d/*
/etc/X11/Xsession.d/*
/var/lib/*
/usr/lib/systemd/system/*
/usr/local/bin/*
/usr/lib/*
"""
    ;;

"fc*_linux_manager" | "suse*_linux_manager")
    echo """/usr/local/bin/*
/usr/share/applications/*
/usr/share/boinc-manager/*
/usr/share/locale/boinc/*
/usr/share/icons/boinc
"""
    ;;

*)  echo "failed"
	;;

esac

exit 0
