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
        return "Advanced GUI";
    } else {
        return "<a href=mac_advanced.php>Advanced GUI</a>";
    }
}

$w419 = array(
    "num"=>"4.19",
    "status"=>"Older version",
    "file"=>"boinc_4.19_windows_intelx86.exe",
    "date"=>"25 Jan 2005",
    "type"=>win_old(),
    null,
    "<ul>
    <li> Doesn't work with some HTTP proxies (fixed in later versions).
    <li>
    If BOINC runs at the same time as Windows XP 3D screensavers,
    the system becomes sluggish and unresponsive.
    </ul>
    "
);

$w443= array(
    "num"=>"4.43",
    "status"=>"Recommended version",
    "file"=>"boinc_4.43_windows_intelx86.exe",
    "date"=>"19 May 2005",
    "type"=>win_new(),
    "features"=>"<ul>
    <li>
    Scheduling within the client software package has been improved to support
    a larger group of projects.
    <li>
    Fixes a bug in which Intellisense/Intellipoint
    closes applications while in screensaver mode.
    <li>
    Fixes bugs with managed deployments with Active Directory.
    <li>
    A new 'Select Computer' dialog remembers which computers you have connected to.
    <li>
    A new statistics tab has been added to the BOINC manager (thanks to
    Jens Breitbart for the great feature).
    <li>
    Support for UMTS networks.
    <li>
    Support for <a href=acct_mgt.php>account management websites</a>.
    <li>
    <a href=gui_rpc.php>Password protection for GUI access</a>.
    <li>
    BOINC Manager now sports a new button based interface which replaces the
    html based interface in previous versions.
    </ul>
    ",
    "bugs"=>"<ul>
    <li>
    Applications that were built before October 2004 do not
    display screensaver graphics with the Service or Shared install type,
    or the Single-user install type with the password protect screensaver
    option on NT based machines.
    <li>
    If BOINC runs at the same time as Windows XP 3D screensavers,
    the system becomes sluggish and unresponsive.
    <li>
    Dial-up users should choose the single-user installation.
    <li>
    Graphics sometimes do not work with ATI Graphics card.
    If you have this problem,
    please visit
    <a href=http://setiweb.ssl.berkeley.edu/forum_thread.php?id=12948>the SETI@home Q&A area</a>,
    fill in the requested information, and click on the
    \"I also have this question\" button.
    </ul>
    If you experience any of these problems,
    we recommend using BOINC version 4.19.
    "
);

$m443 = array(
    "num"=>"4.43",
    "status"=>"Recommended version (advanced GUI)",
    "file"=>"boinc_4.43_macOSX.zip",
    "date"=>"21 May 2005",
    "type"=>mac_advanced(),
    "bugs"=>"<ul>
     <li>
     We have dropped the requirement of our distribution running on 10.2 since 10.4 has been released.  You should still be able to build the client yourself and have it run on a 10.2 machine.
     </ul>
    If you experience any of these problems,
    we recommend using BOINC version 4.19.
    "
);

$m443c = array(
    "num"=>"4.43",
    "status"=>"Recommended version (command line)",
    "file"=>"boinc_4.43_powerpc-apple-darwin.zip",
    "date"=>"21 May 2005",
    "type"=>bare_core(),
    "bugs"=>"
    If you experience any of these problems,
    we recommend using BOINC version 4.19.
    "
);

$m419 = array(
    "num"=>"4.19",
    "status"=>"Old version",
    "file"=>"boinc_4.19_powerpc-apple-darwin.gz",
    "date"=>"25 Jan 2005",
    "type"=>bare_core()
);

$m437s = array(
    "num"=>"4.43 (4)",
    "status"=>"Development version (simple GUI)",
    "file"=>"boinc_menubar_v4.43_(4)_mac.zip",
    "date"=>"23 May 2005",
    "type"=>mac_simple(),
    "bugs"=>"<li>Includes improved BOINC client</li>
     </ul>
    "
);

$l419 = array(
    "num"=>"4.19",
    "status"=>"Recommended version",
    "file"=>"boinc_4.19_i686-pc-linux-gnu.gz",
    "date"=>"25 Jan 2005",
    "type"=>bare_core()
);
$l432 = array(
    "num"=>"4.32",
    "status"=>"Development version",
    "file"=>"boinc_4.32_i686-pc-linux-gnu.sh",
    "date"=>"15 Apr 2005",
    "type"=>sea()
);
$s419 = array(
    "num"=>"4.19",
    "status"=>"Recommended version",
    "file"=>"boinc_4.19_sparc-sun-solaris2.7.gz",
    "date"=>"25 Jan 2005",
    "type"=>bare_core()
);

$s442 = array(
    "num"=>"4.42",
    "status"=>"Development version",
    "file"=>"boinc_4.42_sparc-sun-solaris2.7.sh",
    "date"=>"16 May 2005",
    "type"=>sea()
);

$windows = array(
    "name"=>"Windows",
    "versions"=>array(
        $w443,
        $w419
    )
);
$mac = array(
    "name"=>"Macintosh OS X",
    "versions"=>array(
        $m443,
        $m443c,
        $m437s,
        $m419
    )
);
$linux = array(
    "name"=>"Linux/x86",
    "versions"=>array(
        $l419,
        $l432
    )
);
$solaris = array(
    "name"=>"Solaris/SPARC",
    "versions"=>array(
        $s419,
        $s442
    )
);

$platforms = array(
    "win"=>$windows,
    "mac"=>$mac,
    "linux"=>$linux,
    "solaris"=>$solaris
);

?>
