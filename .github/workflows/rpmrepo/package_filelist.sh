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

case "$1_$2" in
# fedora variants
"fc37_linux_client")
    echo """/etc/boinc-client/*
/etc/default/*
/etc/init.d/*
/etc/bash_completion.d/*
/var/lib/*
/usr/lib/systemd/system/*
/usr/bin/*
/usr/lib/*
"""
    ;;
"fc38_linux_client")
    echo """/etc/boinc-client/*
/etc/default/*
/etc/init.d/*
/etc/bash_completion.d/*
/var/lib/*
/usr/lib/systemd/system/*
/usr/bin/*
/usr/lib/*
"""
    ;;
"fc39_linux_client")
    echo """/etc/boinc-client/*
/etc/default/*
/etc/init.d/*
/etc/bash_completion.d/*
/var/lib/*
/usr/lib/systemd/system/*
/usr/bin/*
/usr/lib/*
"""
    ;;

"fc37_linux_manager")
    echo """/usr/bin/*
/usr/share/applications/*
/usr/share/boinc-manager/*
/usr/share/locale/boinc/*
/usr/share/icons/boinc
"""
    ;;
"fc38_linux_manager")
    echo """/usr/bin/*
/usr/share/applications/*
/usr/share/boinc-manager/*
/usr/share/locale/boinc/*
/usr/share/icons/boinc
"""
    ;;
"fc39_linux_manager")
    echo """/usr/bin/*
/usr/share/applications/*
/usr/share/boinc-manager/*
/usr/share/locale/boinc/*
/usr/share/icons/boinc
"""
    ;;

# suse variants
"suse15_4_linux_client")
    echo """/etc/boinc-client/*
/etc/default/*
/etc/init.d/*
/etc/bash_completion.d/*
/var/lib/*
/usr/lib/systemd/system/*
/usr/bin/*
/usr/lib/*
"""
    ;;
"suse15_5_linux_client")
    echo """/etc/boinc-client/*
/etc/default/*
/etc/init.d/*
/etc/bash_completion.d/*
/var/lib/*
/usr/lib/systemd/system/*
/usr/bin/*
/usr/lib/*
"""
    ;;

"suse15_5_linux_manager")
    echo """/usr/bin/*
/usr/share/applications/*
/usr/share/boinc-manager/*
/usr/share/locale/boinc/*
/usr/share/icons/boinc
"""
    ;;
"suse15_4_linux_manager")
    echo """/usr/bin/*
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
