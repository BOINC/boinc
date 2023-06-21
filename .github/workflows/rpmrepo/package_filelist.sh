#!/bin/bash

case "$1_$2" in
# fedora variants
"fc38_linux_client-vcpkg")
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
"fc37_linux_client-vcpkg")
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

"fc38_linux_manager-without-webview")
    echo """/usr/bin/*
/usr/share/applications/*
/usr/share/boinc-manager/*
/usr/share/locale/boinc/*
/usr/share/icons/boinc
"""
    ;;
"fc37_linux_manager-without-webview")
    echo """/usr/bin/*
/usr/share/applications/*
/usr/share/boinc-manager/*
/usr/share/locale/boinc/*
/usr/share/icons/boinc
"""
    ;;

# suse variants
"suse15_5_linux_client-vcpkg")
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
"suse15_4_linux_client-vcpkg")
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

"suse15_5_linux_manager-without-webview")
    echo """/usr/bin/*
/usr/share/applications/*
/usr/share/boinc-manager/*
/usr/share/locale/boinc/*
/usr/share/icons/boinc
"""
    ;;
"suse15_4_linux_manager-without-webview")
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
