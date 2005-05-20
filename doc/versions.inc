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

$w425 = array(
    "num"=>"4.25",
    "file"=>"boinc_4.25_windows_intelx86.exe",
    "status"=>"Recommended version",
    "date"=>"3 Mar 2005",
    "type"=>win_new(),
    "features"=>"Works with HTTP proxies",
    "bugs"=>"<ul>
    <li/> The BOINC screensaver conflicts with Microsoft Intellitype software.
    <li/>
    Applications that were built before October 2004 do not
    display screensaver graphics with the Service or Shared installation,
    or the Single-user installation with the password protect screensaver
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
    "status"=>"Development version",
    "file"=>"boinc_4.43_windows_intelx86.exe",
    "date"=>"19 May 2005",
    "type"=>win_new(),
    "features"=>"<ul>
    <li>
    A new scheduler has been written for the client-side application so the
    client can be attached to many more without missing deadlines.
    <li>
    Fixes a bug in which Intellisense/Intellipoint
    closes applications while in screensaver mode.
    <li>
    Fixes bugs with managed deployments with Active Directory.
    <li>
    A new 'Select Computer' dialog remembers
    which computers you have connected to.
    <li>
    A new statistics tab has been added to the BOINC manager (thanks to
    Jens Breitbart for the great feature).
    <li>
    Support for UMTS networks.
    <li>
    Numerious usability fixes.
    <li>
    Fixed connectivity problems on all platforms.
    <li>
    Support for <a href=acct_mgt.php>account management websites</a>.
    <li>
    <a href=gui_rpc.php>Password protection for GUI access</a>.
    <li>
    BOINC Manager now sports a new button based interface which replaces the
    html based interface in previous versions.
    <li>
    BOINC Manager now is built with wxWidgets 2.6.0
    <li>
    Symbol files (*.pdb) have been removed from the installer to help out those
    on dialup connections.  We were not receiving very many, if any crashes in 
    the last few months.  We'll make the symbol files available as a seperate
    download.
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
    Same issues for dialup users as 4.25.
    <li>
    Same issues for ATI graphics cards as 4.25.
    </ul>
    "
);

$m419 = array(
    "num"=>"4.19",
    "status"=>"Recommended version",
    "file"=>"boinc_4.19_powerpc-apple-darwin.gz",
    "date"=>"25 Jan 2005",
    "type"=>bare_core()
);

$m437s = array(
    "num"=>"4.37 (3)",
    "status"=>"Development version (simple GUI)",
    "file"=>"boinc_menubar_v4.37_(3)_mac.zip",
    "date"=>"14 May 2005",
    "type"=>mac_simple(),
    "bugs"=>"<li>Includes improved BOINC client</li>
     <li>Fixes a problem which prevented some users from using proxies</li>
     <li>Now displays the current status in the menubar by changing the icon and indicating the amount of work completed</li>
     <li>Fixes a bug on dual processor machines where the status of both processes was not always being displayed </li>
     <li>Adds a preference to share data between users </li><li>Improves security by hiding and encrypting proxy password</li>
     <li>Adds ability to manually run benchmarks</li>
     <li>Improved efficiency</li>
     </ul>
    "
);

$m437 = array(
    "num"=>"4.37",
    "status"=>"Development version (advanced GUI)",
    "file"=>"boinc_4.37_macOSX.zip",
    "date"=>"6 May 2005",
    "type"=>mac_advanced(),
    "bugs"=>"<ul>
     <li>
     We have dropped the requirement of our distribution running on 10.2 since 10.4 has been released.  You should still be able to build the client yourself and have it run on a 10.2 machine.
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
        $w425,
        $w419,
        $w443,
    )
);
$mac = array(
    "name"=>"Macintosh OS X",
    "versions"=>array(
        $m419,
        $m437s,
        $m437
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
