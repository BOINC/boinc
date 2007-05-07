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
        case "mac_advanced":
            return "<a href=mac_advanced.php>Standard GUI</a>";
        }
    }
}

$features_510 = "
    <li> Grid based views.
    <li> Advanced preferences dialog.
    <li> Project list in the attach to project wizard.
    <li> Improved Windows Vista compatibility.
";

$features_57 = "
    <li> BOINC Simple GUI.
    <li> Added memory usage limits for BOINC applications. (Check preferences for new settings)
";

$features_55 = "
    <li> New CPU Scheduler.
    <li> New Work Fetch Policy.
    <li> CPU Capability detection.
    <li> Revamped logging solution.
    <li> Improved security for POSIX compliant systems
         (only implemented for the Mac)
    <li> Add support for file compression on upload.
";

$features_54 = "
    <li>
    BOINC now lets you use 'Account managers' - special web sites
    that let you browse BOINC projects, attach/detach,
    change resource share and settings,
    all with point-and-click simplicity.
    Account managers are also great if you have several computers -
    you just have to make changes once.
    A couple of excellent account managers are nearly ready for use;
    stay tuned to the
    <a href=http://boinc.berkeley.edu/>BOINC web site</a> for details.
    <li>
    Fixes a bug where a personal firewall prevents the BOINC screensaver
    from running, causing your computer to lock up.
    <li>
    General preferences can be overridden by a local file
    details are <a href=http://boinc.berkeley.edu/prefs_override.php>here</a>.
    <li>
    BOINC now alerts you whenever it needs you
    to create a network connection.
    <li> SOCKS5 proxies are now supported.
";

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

$changes_510 = "
";

$changes_57 = "
";

$changes_55 = "
    <li>Changes since 5.5.6
      <ul>
        <li>BOINC Linux build system ovehaul.
        <li>System tray icon changes for suspend/resume operations.
      </ul>
";

$changes_54 = "
    <li>Changes since 5.4.8
      <ul>
        <li>Changed the master url messages from errors to warnings.
      </ul>
    <li>Changes since 5.4.7
      <ul>
        <li>Notify the firewall and security packages of the screensaver's need
            to communicate with the client when the manager launches to avoid
            screensaver lockups due to the firewall and security packages.
        <li>Fix the wizard when running on the Mac so that all the text can
            be displayed.
      </ul>
    <li>Changes since 5.4.6
      <ul>
        <li>Use the correct localized formatting for dates, time, and numerical
            values when displaying messages.
      </ul>
    <li>Changes since 5.4.5
      <ul>
        <li>Attach to project wizard fixed.
        <li>NTLM Authentication for HTTP Proxies disabled.
      </ul>
    <li>Changes since 5.4.4
      <ul>
        <li>More performance enhancements.
        <li>Connection failures due to network issues do not cause a backoff
            event. Retries are set for one minute.
        <li>SOCKS5 authentication bug fixes.  It should now work.
        <li>HTTP authentication bug fixes.  Last round of performance enhancements
            broke http auth.
        <li>Better network tracing through &lt;net_xfer_debug&gt; flag
            in log_flags.xml.
      </ul>
    <li>Changes since 5.4.3
      <ul>
        <li>Performance enhancements.
        <li>Improve dialogs on Linux so they fit well on the screen and have
            OK buttons.
      </ul>
    <li>Changes since 5.4.2
      <ul>
        <li>Fixed a shutdown problem which was causing the manager to linger
            around even after the core client had shutdown.
        <li>Fixed a window flicker problem on application startup.
      </ul>
    <li>Changes since 5.4.1
      <ul>
        <li>Fix a crash when running under Win9x and screen saver passwords
            are enabled.
      </ul>
    <li>Changes since 5.4.0
      <ul>
        <li>Screensaver fixes when running on a machine without a graphics
            accelerator.
        <li>Account manager display issue.
      </ul>
";

$changes_53 = "
    <li>Changes since 5.3.30
      <ul>
        <li>Localization Updates.
        <li>Reworked the service startup code so that DLL initialization happens
            after the call to the service control manager.
        <li>Reorganized the installer components to shrink the installation
            package.
      </ul>
    <li>Changes since 5.3.29
      <ul>
        <li>Localization Updates.
        <li>Fixed BOINC Manager shutdown logic for all platforms.  Previously
              it only worked for Windows.
        <li>Add RPC and SENS dependency to BOINC when running as a service.
      </ul>
    <li>Changes since 5.3.28
      <ul>
        <li>Localization Updates.
        <li>Support for building BOINC Manager with Unicode libraries.
        <li>Fraction done is not checked for until after the app has called
            boinc_fraction_done() at least once. 
        <li>Fix idle detection code on Windows.
        <li>Disable the \"Copy selected message\" button until a message has
            been selected.
        <li>Add additional text to the abort workunit message which displays
            progress and status.
        <li>Don't shutdown a remote BOINC client when exiting the manager.
        <li>Display need to connect to the internet messages for Linux and Mac.
      </ul>
    <li>Changes since 5.3.27
      <ul>
        <li>Localization Updates.
        <li>Account manager fixes when updating preferences.
        <li>Partial file transfer fix.
        <li>Proxy authentication issues on file upload fix.
        <li>Shared install permission bug fix.
      </ul>
    <li>Changes since 5.3.26
      <ul>
        <li>Account Manager fixes.
        <li>Localization Updates.
        <li>BOINC Manager will no longer enable the show graphics button when
            the core client has detected it is running as a service and isn't
            running as the SYSTEM account.
        <li>Preform a scheduler RPC for every project attached via an
            account manager when the client successfully syncs with the
            account manager.
        <li>Fix the crash that would happen on Win9x when shutting down.
      </ul>
    <li>Changes since 5.3.24
      <ul>
        <li>Account Manager fixes.
        <li>Localization Updates.
      </ul>
    <li>Changes since 5.3.23
      <ul>
        <li>Possible fix to work-fetch policy where the client failed to
            get more work before running empty.
        <li>Fix for multiple host creation bug.
        <li>Fix RAM size detection on HPUX.
        <li>Fix name, team, and project name changes in the manager,
        <li>Preserve project directory contents when reattaching to a
            project that uses the anonymous platform mechanism.
        <li>More upload/download display fixes.
      </ul>
    <li>Changes since 5.3.22
      <ul>
        <li>Fix detech from account manager problem.
        <li>Fix multiple boinc.exe execution problems when 
            Fast-User-Switching was used in conjunction with services.
      </ul>
    <li>Changes since 5.3.21
      <ul>
        <li>Partial fix for projects who are already using compression.
      </ul>
    <li>Changes since 5.3.20
      <ul>
        <li>Fixed a few issues with attaching and detaching from projects.
        <li>Fixed an issue with reporting file transfer size.
      </ul>
    <li>Changes since 5.3.16
      <ul>
        <li>Dialup functionality has been streamlined and should support
            a number of different scenarios that the previous scheme did
            not.
        <li>Improve state file recovery logic so that a previous state file
            can be used if the primary state file has become corrupted.
        <li>Introduction of the ability to override the global preferences
            by setting up a global_prefs_override.xml file.
        <li>Increased the refresh rate for viewing data to 1 second from
            5 seconds.
      </ul>
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

$mac_changes_57= "
    <li>Macintosh only:
      <ul>
        <li> Version 5.5.4 BOINC Manager for the Mac introduces new, stricter 
            <a href=sandbox_user.php>security measures</a>.  If you experience  
            problems with this software, you can re-install a version of BOINC 
            prior to 5.5.4; this will automatically revert ownership and permissions 
            to the earlier implementation.  For details, please see the ReadMe file.
        <li> New <a href=mac_admin_tools.php>tools</a> for Macintosh system administrators are available.  
      </ul>
";

$mac_changes_55= "
    <li>Macintosh only:
      <ul>
        <li> Version 5.5.4 BOINC Manager for the Mac introduces new, stricter 
            <a href=sandbox_user.php>security measures</a>.  If you experience  
            problems with this software, you can re-install a version of BOINC 
            prior to 5.5.4; this will automatically revert ownership and permissions 
            to the earlier implementation.  For details, please see the ReadMe file.
        <li> New <a href=mac_admin_tools.php>tools</a> for Macintosh system administrators are available.  
      </ul>
";

$mac_changes_54= "
    <li>Macintosh only:
      <ul>
        <li> To support Intel-based Macs, the Mac BOINC Manager package, 
            stand-alone BOINC client and command-line tool boinc_cmd are 
            now universal binaries, which contain executables for both Intel 
            and PowerPC macs in the same files.  This is indicated by the 
            new download package names: boinc_5.4.0_macOSX_universal.zip and 
            boinc_5.4.0_universal-apple-darwin.zip.  
        <li> We have added a new platform for science project applications 
            to run on Intel Macs: i686-apple-darwin.  The new universal binary 
            Mac clients automatically request applications from the old 
            powerpc-apple-darwin platform when running on a PowerPC Mac, and 
            from the new i686-apple-darwin platform when running on an Intel Mac.
        <li> Upgrade warning: You may lose unreported results if you are upgrading 
            from a version of BOINC earlier than 5.4.0 on an Intel Mac, or if you are 
            transferring BOINC Data from a PowerPC Mac to an Intel Mac, or from an 
            Intel Mac to a PowerPC Mac.  To prevent this, open BOINC Manager window, 
            select the Projects tab, select each project and  press the Update button.  
            Then select the Tasks tab and make sure there are no tasks \"Ready to report.\"            
        <li> We are now building with newer tools and libraries: XCode 2.2, 
            WxWidgets 2.6.2, libcurl 7.15.3.
        <li> Only administrators can attach to a new project
            (for improved security).
        <li> No longer crashes if there is a space character
            in the path to BOINCManager.
      </ul>
";

$linux_libs_58= "
    <li>Boincmgr requires GTK++2.0 or higher, which is included in most
        Linux distributions.  Its been tested on:
      <ul>
        <li>Fedora Core 5
        <li>SUSE 10.1
        <li>Ubuntu 6.06
      </ul>
";

$linux_libs_gcc41= "
    <li>Boincmgr requires GTK++2.0 and glibc 2.4 (libc) or higher which are included
        in recent Linux distributions.  Its been tested on:
      <ul>
        <li>Fedora Core 5 (with updates)
        <li>SUSE 10.1
        <li>Ubuntu 6.10
      </ul>
";

$w599= array(
    "num"=>"5.9.9",
    "status"=>"Development version",
    "file"=>"boinc_5.9.9_windows_intelx86.exe",
    "date"=>"7 May 2007",
    "type"=>"win_new",
    "features"=>"<ul>
".$features_510."
".$changes_510."
    </ul>
    ",
    "bugs"=>""
);

$w599x64= array(
    "num"=>"5.9.9",
    "status"=>"Development version",
    "file"=>"boinc_5.9.9_windows_x86_64.exe",
    "date"=>"7 May 2007",
    "type"=>"win_new",
    "features"=>"<ul>
".$features_510."
".$changes_510."
    </ul>
    ",
    "bugs"=>""
);

$w593= array(
    "num"=>"5.9.3",
    "status"=>"Development version",
    "file"=>"boinc_5.9.3_windows_intelx86.exe",
    "date"=>"4 Apr 2007",
    "type"=>"win_new",
    "features"=>"<ul>
".$features_510."
".$changes_510."
    </ul>
    ",
    "bugs"=>""
);

$w5816= array(
    "num"=>"5.8.16",
    "status"=>"Recommended version",
    "file"=>"boinc_5.8.16_windows_intelx86.exe",
    "date"=>"1 Mar 2007",
    "type"=>"win_new",
    "features"=>"<ul>
".$features_55.$features_57."
".$changes_57."
    </ul>
    ",
    "bugs"=>""
);

$w5411= array(
    "num"=>"5.4.11",
    "status"=>"Older version",
    "file"=>"boinc_5.4.11_windows_intelx86.exe",
    "date"=>"2 Aug 2006",
    "type"=>"win_new",
    "features"=>"<ul>
".$features_54."
".$changes_54."
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
    If you use a proxy server, please hold off upgrading for now.  We have
    a fix in the works for proxies that use NTLM authentication.
    </ul>
    "
);

$m599 = array(
    "num"=>"5.9.9",
    "status"=>"Development version (standard GUI)",
    "file"=>"boinc_5.9.9_macOSX_universal.zip",
    "date"=>"7 May 2007",
    "type"=>"mac_advanced",
     "features"=>"<ul>
".$features_510."
".$changes_510."
     </ul>
    ",
    "bugs"=>"
    "
);

$m5817 = array(
    "num"=>"5.8.17",
    "status"=>"Recommended version (standard GUI)",
    "file"=>"boinc_5.8.17_macOSX_universal.zip",
    "date"=>"3 Apr 2007",
    "type"=>"mac_advanced",
     "features"=>"<ul>
".$mac_changes_57."
".$features_55."
".$features_57."
".$changes_57."
     </ul>
    ",
    "bugs"=>"
    "
);

$m549 = array(
    "num"=>"5.4.9",
    "status"=>"Older version (standard GUI)",
    "file"=>"boinc_5.4.9_macOSX_universal.zip",
    "date"=>"5 May 2006",
    "type"=>"mac_advanced",
     "features"=>"<ul>
".$features_54."
".$mac_changes_54."
".$changes_54."
".$changes_53."
     <li>
     BOINC now works with OS 10.3.0 or higher.
     <li>
     Closing the BOINC Manager window now hides the application, and vice-versa.
     <li>
     The installer no longer always sets ownership to user 501. If the installing user 
     has admin privileges, that user becomes the owner; otherwise the first non-root 
     admin user becomes the owner.
     </ul>
    ",
    "bugs"=>"
    "
);

$m599c = array(
    "num"=>"5.9.9",
    "status"=>"Development version (Unix command-line version)",
    "file"=>"boinc_5.9.9_universal-apple-darwin.zip",
    "date"=>"7 May 2007",
    "type"=>"bare_core",
    "features"=>"<ul>
".$features_510."
".$changes_510."
     </ul>
    ",
    "bugs"=>"
    "
);

$m5817c = array(
    "num"=>"5.8.17",
    "status"=>"Recommended version (Unix command-line version)",
    "file"=>"boinc_5.8.17_universal-apple-darwin.zip",
    "date"=>"3 Apr 2007",
    "type"=>"bare_core",
    "features"=>"<ul>
".$mac_changes_57."
".$features_55."
".$features_57."
".$changes_57."
     </ul>
    ",
    "bugs"=>"
    "
);

$m549c = array(
    "num"=>"5.4.9",
    "status"=>"Older version (Unix command-line version)",
    "file"=>"boinc_5.4.9_universal-apple-darwin.zip",
    "date"=>"5 May 2006",
    "type"=>"bare_core",
    "features"=>"<ul>
".$features_54."
".$mac_changes_54."
".$changes_54."
".$changes_53."
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

$l595 = array(
    "num"=>"5.9.5",
    "status"=>"Development version (standard GUI)",
    "file"=>"boinc_5.9.5_i686-pc-linux-gnu.sh",
    "date"=>"01 May 2007",
    "type"=>"sea",
    "features"=>"<ul>
".$features_510."
".$linux_libs_gcc41." 
".$changes_510."
    </ul>
    "
);
$l5816 = array(
    "num"=>"5.8.16",
    "status"=>"Recommended version (standard GUI)",
    "file"=>"boinc_5.8.16_i686-pc-linux-gnu.sh",
    "date"=>"7 March 2007",
    "type"=>"sea",
    "features"=>"<ul>
".$features_55.$features_57."
".$changes_57."
    </ul>
    "
);

$l5411 = array(
    "num"=>"5.4.11",
    "status"=>"Recommended version",
    "file"=>"boinc_5.4.11_i686-pc-linux-gnu.sh",
    "date"=>"12 December 2006",
    "type"=>"sea",
    "features"=>"<ul>
".$features_54."
".$changes_54."
    </ul>
    "
);

$windows = array(
    "name"=>"Windows",
    "dbname" => "windows_intelx86",
    "versions"=>array(
        $w599,
        $w5816,
        $w5411,
    )
);

$windowsx64 = array(
    "name"=>"Windows x64",
    "dbname" => "windows_x86_64",
    "versions"=>array(
        $w599x64,
    )
);

$mac = array(
    "name"=>"Mac OS X",
    "dbname" => "powerpc-apple-darwin",
    "versions"=>array(
        $m599,
        $m599c,
        $m5817,
        $m5817c,
        $m549,
        $m549c,
    )
);
$linux = array(
    "name"=>"Linux/x86",
    "dbname" => "i686-pc-linux-gnu",
    "url"=>"linux.php",
    "versions"=>array(
        $l595,
        $l5816,
        $l5411,
    )
);

$platforms = array(
    "win"=>$windows,
    "winx64"=>$windowsx64,
    "mac"=>$mac,
    "linux"=>$linux,
);

function is_dev($v) {
    return (strstr($v["status"], "Development") != null);
}

switch(rand(0,5)) {
    case 0: $url_base = "http://boinc.berkeley.edu/dl/"; break;
    case 1: $url_base = "http://morel.mit.edu/download/boinc/dl/"; break;
    case 2: $url_base = "http://einstein.aei.mpg.de/download/boinc/dl/"; break;
    case 3: $url_base = "http://einstein.astro.gla.ac.uk/download/boinc/dl/"; break;
    case 4: $url_base = "http://einstein.aset.psu.edu/download/boinc/dl/"; break;
    case 5: $url_base = "http://einstein.ligo.caltech.edu/download/boinc/dl/"; break;
    //case 6: $url_base = "http://einstein.phys.uwm.edu/download/boinc/dl/"; break;
}

?>
