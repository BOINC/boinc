<?php

function type_text($type) {
    global $xml;
    if ($xml) {
        switch($type) {
        case "win_old":
            return "Single-mode Windows installer";
        case "win_new":
            return "Windows installer";
        case "bare_core":
            return "Core client only (command-line)";
        case "sea":
            return "Self-extracting archive";
        case "mac_simple":
            return "Menubar (simple GUI)";
        case "mac_advanced":
            return "Standard GUI";
        }
    } else {
        switch($type) {
        case "win_old":
            return "<a href=client_windows.php>Single-mode Windows installer</a>";
        case "win_new":
            return "<a href=win_install.php>Windows installer</a>";
        case "bare_core":
            return "<a href=bare_core.php>Core client only (command-line)</a>";
        case "sea":
            return "<a href=sea.php>Self-extracting archive</a>";
        case "mac_simple":
            return "<a href=menubar.php>Menubar (simple GUI)</a>";
        case "mac_advanced":
            return "<a href=mac_advanced.php>Standard GUI</a>";
        }
    }
}

$features_53 = "
    <li> Customized clients can now be created.
    <li> Account manager support has been enhanced.
    <li> Farm manager support has been added.
";

$features_52 = "
    <li> You attach to projects using your own email address and password.
        Long, random 'account keys' are no longer used.
    <li> BOINC checks for proxy configuration problems
        when you first attach to a project.
    <li>
    The file gui_rpc_auth.cfg contains a password that
    protects the BOINC client from control by other users on the same host.
    BOINC automatically generates a password if none is found.
    If you use tools like BOINCView you may need to look it up or change it.
    If you need to revert from 5.2.x to an earlier version,
    you must delete gui_rpc_auth.cfg from your BOINC directory.
    <li> Alert boxes now provide feedback when certain errors occur.
    <li> BOINC supports 'Account Management Systems' (to be announced).
";

$changes_53 = "
    <li>Changes since 5.3.15
      <ul>
        <li>CPU scheduler reverted back to previous scheme.
        <li>Item selection on Linux and Mac fixed in list view controls.
      </ul>
";

$changes_52 = "
    <li>Changes since 5.2.12
      <ul>
        <li>Localization updates.
        <li>Network communication failure page added which describes basic ways to
            resolve common networking issues.
        <li>Fixed a timing issue where the manager would look for the password file
            before the client actually created one.
      </ul>
    <li>Changes since 5.2.11
      <ul>
        <li>Prompt to retry the connection if a connection failed event occurs.
        <li>Treat various forms of the local computer name as a local machine instead
            of a remote machine.
      </ul>
    <li>Changes since 5.2.10
      <ul>
        <li>Wizards cannot be run without a connection to the core client.
      </ul>
    <li>Changes since 5.2.9
      <ul>
        <li>Win: Fixes a BOINC Manager connection issue when the manager is
            waiting on a firewall for the all clear.  This generally happens
            after a reboot.
      </ul>
    <li>Changes since 5.2.8
      <ul>
        <li>Win: Fixes the 'connection failed' message when another program
            is using the BOINC communication port.
      </ul>
    <li>Changes since 5.2.7
      <ul>
        <li>Localization updates.
        <li>Resolve many connection issues regarding passwords.
        <li>Mac: Fix a screensaver stutter while scrolling text.
      </ul>
    <li>Changes since 5.2.6
      <ul>
        <li>Localization updates.
        <li>Usability enhancements.
        <li>Fix a crash condition when connecting to a remote machine.
        <li>Fix for file handle usage on Linux/Mac which caused BOINC to run out of
            file handles.
      </ul>
";

$mac_changes_53= "
    <li>Macintosh only:
      <ul>
        <li> We are now building with newer tools and libraries: XCode 2.2, 
            WxWidgets 2.6.2, libcurl 7.15.1.
        <li> In anticipation of Intel-based Macs, the Mac BOINC Manager package 
            name now specifies the CPU architecture:
            boinc_5.3.16_macOSX_powerpc.zip.
        <li> Only administrators can attach to a new project
            (for improved security).
        <li> No longer crashes if there is a space character
            in the path to BOINCManager.
      </ul>
";

$w5316= array(
    "num"=>"5.3.16",
    "status"=>"Development version",
    "file"=>"boinc_5.3.16_windows_intelx86.exe",
    "date"=>"30 Jan 2006",
    "type"=>"win_new",
    "features"=>"<ul>
".$features_53."
".$changes_53."
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

$w5213= array(
    "num"=>"5.2.13",
    "status"=>"Recommended version",
    "file"=>"boinc_5.2.13_windows_intelx86.exe",
    "date"=>"29 Nov 2005",
    "type"=>"win_new",
    "features"=>"<ul>
    <li> Dial-up features now work with all installation types.
    <li> The BOINC Manager can now be controlled using only the keyboard.
".$features_52."
".$changes_52."
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
    "type"=>"win_new",
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
    "type"=>"win_old",
    null,
    "bugs"=>"<ul>
    <li> Doesn't work with some HTTP proxies (fixed in later versions).
    <li>
    If BOINC runs at the same time as Windows XP 3-D screensavers,
    the system becomes sluggish.
    </ul>
    "
);

$m5316 = array(
    "num"=>"5.3.16",
    "status"=>"Development version (standard GUI)",
    "file"=>"boinc_5.3.16_macOSX_powerpc.zip",
    "date"=>"31 January 2006",
    "type"=>"mac_advanced",
     "features"=>"<ul>
     <li>
     BOINC now works with OS 10.3.0 or higher.
     <li>
     Closing the BOINC Manager window now hides the application, and vice-versa.
     <li>
     The installer no longer always sets ownership to user 501. If the installing user 
     has admin privileges, that user becomes the owner; otherwise the first non-root 
     admin user becomes the owner.
".$features_53."
".$mac_changes_53."
".$changes_53."
     </ul>
    ",
    "bugs"=>"
    "
);

$m5213 = array(
    "num"=>"5.2.13",
    "status"=>"Recommended version (standard GUI)",
    "file"=>"boinc_5.2.13_macOSX.zip",
    "date"=>"29 November 2005",
    "type"=>"mac_advanced",
     "features"=>"<ul>
     <li>
     BOINC now works with OS 10.3.0 or higher.
     <li>
     Closing the BOINC Manager window now hides the application, and vice-versa.
     <li>
     The installer no longer always sets ownership to user 501. If the installing user 
     has admin privileges, that user becomes the owner; otherwise the first non-root 
     admin user becomes the owner.
".$features_52."
".$changes_52."
     </ul>
    ",
    "bugs"=>"
    "
);

$m5213c = array(
    "num"=>"5.2.13",
    "status"=>"Unix command-line version",
    "file"=>"boinc_5.2.13_powerpc-apple-darwin.zip",
    "date"=>"29 November 2005",
    "type"=>"bare_core",
    "features"=>"<ul>
     <li>
     BOINC now works with OS 10.3.0 or higher.
    <li>
    We are now including a tool with command line interface to control boinc within the 
    default distribution
    </ul>
    ",
    "bugs"=>"
    "
);

$m5213s = array(
    "num"=>"5.2.13",
    "status"=>"Recommended version (simple GUI)",
    "file"=>"boinc_menubar_v5.2.13_mac.zip",
    "date"=>"29 November 2005",
    "type"=>"mac_simple",
    "features"=>"<ul><li>Includes improved BOINC client
     </ul>
    "
);

$m443 = array(
    "num"=>"4.43",
    "status"=>"Older version (standard GUI)",
    "file"=>"boinc_4.43_macOSX.zip",
    "date"=>"21 May 2005",
    "type"=>"mac_advanced",
);

$m443c = array(
    "num"=>"4.43",
    "status"=>"Older version (Unix command-line version)",
    "file"=>"boinc_4.43_powerpc-apple-darwin.zip",
    "date"=>"21 May 2005",
    "type"=>"bare_core",
    "bugs"=>"
    "
);

$m443s = array(
    "num"=>"4.43.4",
    "status"=>"Older version (simple GUI)",
    "file"=>"boinc_menubar_v4.43_(4)_mac.zip",
    "date"=>"23 May 2005",
    "type"=>"mac_simple",
    "features"=>"<ul><li>Includes improved BOINC client
     </ul>
    "
);

$l5316 = array(
    "num"=>"5.3.16",
    "status"=>"Development version",
    "file"=>"boinc_5.3.16_i686-pc-linux-gnu.sh",
    "date"=>"30 Jan 2006",
    "type"=>"sea",
    "features"=>"<ul>
".$features_53."
".$changes_53."
    </ul>
    "
);

$l5213 = array(
    "num"=>"5.2.13",
    "status"=>"Recommended version",
    "file"=>"boinc_5.2.13_i686-pc-linux-gnu.sh",
    "date"=>"29 Nov 2005",
    "type"=>"sea",
    "features"=>"<ul>
".$features_52."
".$changes_52."
    </ul>
    "
);

$l443 = array(
    "num"=>"4.43",
    "status"=>"Older version",
    "file"=>"boinc_4.43_i686-pc-linux-gnu.sh",
    "date"=>"23 May 2005",
    "type"=>"sea",
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
    "type"=>"bare_core",
);

$s419 = array(
    "num"=>"4.19",
    "status"=>"Older version",
    "file"=>"boinc_4.19_sparc-sun-solaris2.7.gz",
    "date"=>"25 Jan 2005",
    "type"=>"bare_core",
);

$s443 = array(
    "num"=>"4.43",
    "status"=>"Recommended version",
    "file"=>"boinc_4.43_sparc-sun-solaris2.7.sh",
    "date"=>"26 May 2005",
    "type"=>"sea",
);

$windows = array(
    "name"=>"Windows",
    "dbname" => "windows_intelx86",
    "versions"=>array(
        $w5316,
        $w5213,
        $w445,
    )
);
$mac = array(
    "name"=>"Macintosh OS X (10.3 or higher)",
    "dbname" => "powerpc-apple-darwin",
    "versions"=>array(
        $m5316,
        $m5213,
        $m5213s,
        $m5213c,
        $m443,
        $m443c,
        $m443s
    )
);
$linux = array(
    "name"=>"Linux/x86",
    "dbname" => "i686-pc-linux-gnu",
    "url"=>"linux.php",
    "versions"=>array(
        $l5316,
        $l5213,
        $l443,
    )
);
$solaris = array(
    "name"=>"Solaris/SPARC",
    "dbname" => "sparc-sun-solaris2.7",
    "url"=>"solaris.php",
    "versions"=>array(
        $s443,
        $s419,
    )
);

$platforms = array(
    "win"=>$windows,
    "mac"=>$mac,
    "linux"=>$linux,
    "solaris"=>$solaris
);

?>
