<?php

function win_old() {
    global $xml;
    if ($xml) {
        return "Single-mode Windows installer";
    } else {
        return "<a href=client_windows.php>Single-mode Windows installer</a>";
    }
}

function win_new() {
    global $xml;
    if ($xml) {
        return "Windows installer";
    } else {
        return "<a href=win_install.php>Windows installer</a>";
    }
}

function bare_core() {
    global $xml;
    if ($xml) {
        return "Core client only (command-line)";
    } else {
        return "<a href=bare_core.php>Core client only (command-line)</a>";
    }
}

function sea() {
    global $xml;
    if ($xml) {
        return "Self-extracting archive";
    } else {
        return "<a href=sea.php>Self-extracting archive</a>";
    }
}

function mac_simple() {
    global $xml;
    if ($xml) {
        return "Menubar (simple GUI)";
    } else {
        return "<a href=menubar.php>Menubar (simple GUI)</a>";
    }
}

function mac_advanced() {
    global $xml;
    if ($xml) {
        return "Standard GUI";
    } else {
        return "<a href=mac_advanced.php>Standard GUI</a>";
    }
}

$w522= array(
    "num"=>"5.2.2",
    "status"=>"Recommended version",
    "file"=>"boinc_5.2.2_windows_intelx86.exe",
    "date"=>"17 October 2005",
    "type"=>win_new(),
    "features"=>"<ul>
    <li>
    Dial-up support has now been enhanced to support service installs, and shared user installs.
    <li>
    Account Management has been enhanced and support has been enabled in the stock install of BOINC.
    <li>
    Alerts have been enabled to provide instant feedback when certain types of errors occur.
    <li>
    BOINC Manager now supports the Microsoft Active Accessibility standard.
    <li>
    BOINC is now capable of using HTTPS and has better proxy support through the use of curl.
    <li>
    BOINC can now use username and passwords for authentication instead of an authenticator.
    <li>
    BOINC now employs a wizard during an attach to project operation to guide people thruogh the
    process of attaching to a project as well as check for things like proxy configuration issues.
    <li>
    BOINC now automatically generates a password that is used for network communication.  If you
    use programs like BOINCView or any other third party tool you may need to look up or change
    the password specified in gui_rpc_auth.cfg.  This change keeps people from randomly detaching
    from projects over the network on remote machines.
    <li>
    If you need to revert from 5.2.x to an earlier version, you must delete the file
    gui_rpc_auth.cfg from your BOINC directory.
    </ul>
    ",
    "bugs"=>"<ul>
    <li>
    Applications that were built before October 2004 do not
    display screensaver graphics with the Service or Shared install type,
    or the Single-user install type with the password protect screensaver
    option on NT based machines.
    <li>
    If BOINC runs at the same time as Windows XP 3-D screensavers,
    the system becomes sluggish and unresponsive.
    </ul>
    "
);

$w445= array(
    "num"=>"4.45",
    "status"=>"Older version",
    "file"=>"boinc_4.45_windows_intelx86.exe",
    "date"=>"6 June 2005",
    "type"=>win_new(),
    "features"=>"<ul>
    <li>
    We are now including a tool with command line interface to control boinc within the 
    default distribution
    </ul>
    ",
    "features"=>"<ul>
    <li>
    When a slow computer is attached to a large number of projects,
    BOINC does work only for a few of them at a time,
    to avoid missed deadlines.
    <li>
    Fixes a bug in which Intellisense/Intellipoint
    closes applications while in screensaver mode.
    <li>
    Fixes bugs with managed deployments with Active Directory.
    <li>
    A new 'Select Computer' dialog remembers which computers you have connected to.
    <li>
    A new statistics tab has been added to the BOINC manager
    (thanks to Jens Breitbart for the great feature).
    <li>
    Support for UMTS networks.
    <li>
    Support for <a href=acct_mgt.php>account management websites</a>.
    <li>
    <a href=gui_rpc.php>Password protection for GUI access</a>.
    <li>
    BOINC Manager now uses buttons instead of web-type links.
    </ul>
    ",
    "bugs"=>"<ul>
    <li>
    Applications that were built before October 2004 do not
    display screensaver graphics with the Service or Shared install type,
    or the Single-user install type with the password protect screensaver
    option on NT based machines.
    <li>
    If BOINC runs at the same time as Windows XP 3-D screensavers,
    the system becomes sluggish and unresponsive.
    <li>
    Dial-up users should choose the single-user installation.
    <li>
    Graphics sometimes do not work with ATI Graphics card.
    This bug has been fixed, but it requires new applications to be built.  Please check 
    with the various projects to find out when they'll release a new applicaton with the fix.
    </ul>
    "
);

$w419 = array(
    "num"=>"4.19",
    "status"=>"Older version",
    "file"=>"boinc_4.19_windows_intelx86.exe",
    "date"=>"25 Jan 2005",
    "type"=>win_old(),
    null,
    "bugs"=>"<ul>
    <li> Doesn't work with some HTTP proxies (fixed in later versions).
    <li>
    If BOINC runs at the same time as Windows XP 3-D screensavers,
    the system becomes sluggish.
    </ul>
    "
);

$m443 = array(
    "num"=>"4.43",
    "status"=>"Recommended version (standard GUI)",
    "file"=>"boinc_4.43_macOSX.zip",
    "date"=>"21 May 2005",
    "type"=>mac_advanced(),
    "bugs"=>"<ul>
     <li>
     BOINC no longer works on Mac OS 10.2 and requires 10.3.9 or higher.
     </ul>
    "
);

$m443c = array(
    "num"=>"4.43",
    "status"=>"Unix command-line version",
    "file"=>"boinc_4.43_powerpc-apple-darwin.zip",
    "date"=>"21 May 2005",
    "type"=>bare_core(),
    "bugs"=>"
    "
);

$m419 = array(
    "num"=>"4.19",
    "status"=>"Older version",
    "file"=>"boinc_4.19_powerpc-apple-darwin.gz",
    "date"=>"25 Jan 2005",
    "type"=>bare_core()
);

$m437s = array(
    "num"=>"4.43 (4)",
    "status"=>"Recommended version (simple GUI)",
    "file"=>"boinc_menubar_v4.43_(4)_mac.zip",
    "date"=>"23 May 2005",
    "type"=>mac_simple(),
    "features"=>"<ul><li>Includes improved BOINC client
     </ul>
    "
);

$m524 = array(
    "num"=>"5.2.4",
    "status"=>"Development version (standard GUI)",
    "file"=>"boinc_5.2.4_macOSX.zip",
    "date"=>"20 October 2005",
    "type"=>mac_advanced(),
     "features"=>"<ul>
     <li>
     BOINC now works with OS 10.3.0 or higher.
     <li>
     Closing the BOINC Manager window now hides the application, and vice-versa.
     <li>
     The installer no longer always sets ownership to user 501. If the installing user 
     has admin privileges, that user becomes the owner; otherwise the first non-root 
     admin user becomes the owner.
    <li>
    Account Management has been enhanced and support has been enabled in the stock install of BOINC.
    <li>
    Alerts have been enabled to provide instant feedback when certain types of errors occur.
    <li>
    BOINC is now capable of using HTTPS and has better proxy support through the use of curl.
    <li>
    BOINC can now use username and passwords for authentication instead of an authenticator.
    <li>
    BOINC now employs a wizard during an attach to project operation to guide people thruogh the
    process of attaching to a project as well as check for things like proxy configuration issues.
    <li>
    BOINC now automatically generates a password that is used for network communication.  If you
    use programs like BOINCView or any other third party tool you may need to look up or change
    the password specified in gui_rpc_auth.cfg.  This change keeps people from randomly detaching
    from projects over the network on remote machines.
    <li>
    If you need to revert from 5.2.x to an earlier version, you must delete the file
    gui_rpc_auth.cfg from your BOINC directory.
     </ul>
    ",
    "bugs"=>"
    "
);

$m524c = array(
    "num"=>"5.2.4",
    "status"=>"Development version (Unix command-line version)",
    "file"=>"boinc_5.2.4_powerpc-apple-darwin.zip",
    "date"=>"20 October 2005",
    "type"=>bare_core(),
    "features"=>"<ul>
    <li>
    We are now including a tool with command line interface to control boinc within the 
    default distribution
    </ul>
    ",
    "bugs"=>"
    "
);

$l524 = array(
    "num"=>"5.2.4",
    "status"=>"Recommended version",
    "file"=>"boinc_5.2.4_i686-pc-linux-gnu.sh",
    "date"=>"19 October 2005",
    "type"=>sea(),
    "features"=>"<ul>
    <li>
    Account Management has been enhanced and support has been enabled in the stock install of BOINC.
    <li>
    Alerts have been enabled to provide instant feedback when certain types of errors occur.
    <li>
    BOINC is now capable of using HTTPS and has better proxy support through the use of curl.
    <li>
    BOINC can now use username and passwords for authentication instead of an authenticator.
    <li>
    BOINC now employs a wizard during an attach to project operation to guide people thruogh the
    process of attaching to a project as well as check for things like proxy configuration issues.
    <li>
    The installer now contains the various BOINC localization files.
    <li>
    BOINC now automatically generates a password that is used for network communication.  If you
    use programs like BOINCView or any other third party tool you may need to look up or change
    the password specified in gui_rpc_auth.cfg.  This change keeps people from randomly detaching
    from projects over the network on remote machines.
    <li>
    If you need to revert from 5.2.x to an earlier version, you must delete the file
    gui_rpc_auth.cfg from your BOINC directory.
    </ul>
    "
);

$l443 = array(
    "num"=>"4.43",
    "status"=>"Older version",
    "file"=>"boinc_4.43_i686-pc-linux-gnu.sh",
    "date"=>"23 May 2005",
    "type"=>sea(),
    "bugs"=>"<ul>
     <li>
     boincmgr currently requires a later version of glibc than what we want.  When running
     boincmgr you might see this error message:<br>
         boincmgr: /lib/libpthread.so.0: version `GLIBC_2.3.2' not found (required by boincmgr)<br><br>
     We are investigating this issue.
     <li>
     Cutting and pasting does not appear to work.
     </ul>
    "
);

$l419 = array(
    "num"=>"4.19",
    "status"=>"Older version",
    "file"=>"boinc_4.19_i686-pc-linux-gnu.gz",
    "date"=>"25 Jan 2005",
    "type"=>bare_core()
);

$s419 = array(
    "num"=>"4.19",
    "status"=>"Recommended version",
    "file"=>"boinc_4.19_sparc-sun-solaris2.7.gz",
    "date"=>"25 Jan 2005",
    "type"=>bare_core()
);

$s443 = array(
    "num"=>"4.43",
    "status"=>"Development version",
    "file"=>"boinc_4.43_sparc-sun-solaris2.7.sh",
    "date"=>"26 May 2005",
    "type"=>sea()
);

$windows = array(
    "name"=>"Windows",
    "versions"=>array(
        $w522,
        $w445,
        $w419
    )
);
$mac = array(
    "name"=>"Macintosh OS X (10.3 or higher)",
    "versions"=>array(
        $m524,
        $m524c,
        $m443,
        $m437s,
        $m443c,
        $m419
    )
);
$linux = array(
    "name"=>"Linux/x86",
    "url"=>"linux.php",
    "versions"=>array(
        $l524,
        $l443,
        $l419
    )
);
$solaris = array(
    "name"=>"Solaris/SPARC",
    "url"=>"solaris.php",
    "versions"=>array(
        $s419,
        $s443
    )
);

$platforms = array(
    "win"=>$windows,
    "mac"=>$mac,
    "linux"=>$linux,
    "solaris"=>$solaris
);

?>
