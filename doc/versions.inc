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

$features_55 = "
    <li> New CPU Scheduler.
    <li> CPU Capability detection.
    <li> Revamped logging solution.
    <li> Improved security for posix compliant systems. (Right now only 
         implemented for the Mac)
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

$mac_changes_55= "
    <li>Macintosh only:
      <ul>
        <li> Version 5.5.4 BOINC Manager for the Mac introduces new, stricter 
            security measures.  If you experience problems with this software, 
            you can re-install a version of BOINC prior to 5.5.4; this will 
            automatically revert ownership and permissions to the earlier 
            implementation.  For details, please see the ReadMe file.
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

$linux_libs_fedora_559= "
    <li>Boincmgr no longer use statically linked libraries.
        The following shared libraries are needed for boincmgr to run:
      <ul>
        <li>/lib/ld-linux.so.2
        <li>libatk-1.0.so.0
        <li>libc.so.6
        <li>libcairo.so.2
        <li>libdl.so.2
        <li>libexpat.so.0
        <li>libfontconfig.so.1
        <li>libfreetype.so.6
        <li>libgcc_s.so.1
        <li>libgdk-x11-2.0.so.0
        <li>libgdk_pixbuf-2.0.so.0
        <li>libglib-2.0.so.0
        <li>libgmodule-2.0.so.0
        <li>libgobject-2.0.so.0
        <li>libgthread-2.0.so.0
        <li>libgtk-x11-2.0.so.0
        <li>libm.so.6
        <li>libpango-1.0.so.0
        <li>libpangocairo-1.0.so.0
        <li>libpangoft2-1.0.so.0
        <li>libpng12.so.0
        <li>libpthread.so.0
        <li>libX11.so.6
        <li>libXau.so.6
        <li>libXcursor.so.1
        <li>libXdmcp.so.6
        <li>libXext.so.6
        <li>libXfixes.so.3
        <li>libXi.so.6
        <li>libXinerama.so.1
        <li>libXrandr.so.2
        <li>libXrender.so.1
        <li>libXxf86vm.so.1
        <li>libz.so.1
        <li>linux-gate.so.1
      </ul>
";

$linux_libs_suse_559= "
    <li>Boincmgr no longer use statically linked libraries.
        The following shared libraries are needed for boincmgr to run:
      <ul>
        <li>/lib/ld-linux.so.2
        <li>libatk-1.0.so.0
        <li>libc.so.6
        <li>libcairo.so.2
        <li>libdl.so.2
        <li>libexpat.so.1
        <li>libfontconfig.so.1
        <li>libfreetype.so.6
        <li>libgcc_s.so.1
        <li>libgdk-x11-2.0.so.0
        <li>libgdk_pixbuf-2.0.so.0
        <li>libglib-2.0.so.0
        <li>libglitz.so.1
        <li>libgmodule-2.0.so.0
        <li>libgobject-2.0.so.0
        <li>libgthread-2.0.so.0
        <li>libgtk-x11-2.0.so.0
        <li>libm.so.6
        <li>libpango-1.0.so.0
        <li>libpangocairo-1.0.so.0
        <li>libpangoft2-1.0.so.0
        <li>libpng12.so.0
        <li>libpthread.so.0
        <li>libX11.so.6
        <li>libXcursor.so.1
        <li>libXext.so.6
        <li>libXfixes.so.3
        <li>libXi.so.6
        <li>libXinerama.so.1
        <li>libXrandr.so.2
        <li>libXrender.so.1
        <li>libXxf86vm.so.1
        <li>libz.so.1
        <li>linux-gate.so.1
      </ul>
";

$linux_libs_ubuntu_559= "
    <li>Boincmgr no longer use statically linked libraries.
        The following shared libraries are needed for boincmgr to run:
      <ul>
        <li>/lib/ld-linux.so.2
        <li>libatk-1.0.so.0
        <li>libc.so.6
        <li>libcairo.so.2
        <li>libdl.so.2
        <li>libexpat.so.1
        <li>libfontconfig.so.1
        <li>libfreetype.so.6
        <li>libgcc_s.so.1
        <li>libgdk-x11-2.0.so.0
        <li>libgdk_pixbuf-2.0.so.0
        <li>libglib-2.0.so.0
        <li>libgmodule-2.0.so.0
        <li>libgobject-2.0.so.0
        <li>libgthread-2.0.so.0
        <li>libgtk-x11-2.0.so.0
        <li>libm.so.6
        <li>libpango-1.0.so.0
        <li>libpangocairo-1.0.so.0
        <li>libpangoft2-1.0.so.0
        <li>libpng12.so.0
        <li>libpthread.so.0
        <li>libX11.so.6
        <li>libXau.so.6
        <li>libXcursor.so.1
        <li>libXext.so.6
        <li>libXfixes.so.3
        <li>libXi.so.6
        <li>libXinerama.so.1
        <li>libXrandr.so.2
        <li>libXrender.so.1
        <li>libz.so.1
        <li>linux-gate.so.1
      </ul>
";

$w559= array(
    "num"=>"5.5.9",
    "status"=>"Development version",
    "file"=>"boinc_5.5.9_windows_intelx86.exe",
    "date"=>"26 July 2006",
    "type"=>"win_new",
    "features"=>"<ul>
".$features_55."
".$changes_55."
    </ul>
    ",
    "bugs"=>"<ul>
    <li>
    If you use a proxy server, please hold off upgrading for now.  We have
    a fix in the works for proxies that use NTLM authentication.
    </ul>
    "
);

$w549= array(
    "num"=>"5.4.9",
    "status"=>"Recommended version",
    "file"=>"boinc_5.4.9_windows_intelx86.exe",
    "date"=>"5 May 2006",
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

$w5213= array(
    "num"=>"5.2.13",
    "status"=>"Older version",
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
    with the various projects to find out when they'll release a new application with the fix.
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

$m559 = array(
    "num"=>"5.5.9",
    "status"=>"Development version (standard GUI)",
    "file"=>"boinc_5.5.9_macOSX_universal.zip",
    "date"=>"26 July 2006",
    "type"=>"mac_advanced",
     "features"=>"<ul>
".$mac_changes_55."
".$features_55."
".$changes_55."
".$features_54."
".$mac_changes_54."
".$features_53."
".$mac_changes_53."
".$changes_53."
     </ul>
    ",
    "bugs"=>"
    "
);

$m549 = array(
    "num"=>"5.4.9",
    "status"=>"Recommended version (standard GUI)",
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

$m5213 = array(
    "num"=>"5.2.13",
    "status"=>"Older version (standard GUI)",
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

$m559c = array(
    "num"=>"5.5.9",
    "status"=>"Development version (Unix command-line version)",
    "file"=>"boinc_5.5.9_universal-apple-darwin.zip",
    "date"=>"26 July 2006",
    "type"=>"bare_core",
    "features"=>"<ul>
".$mac_changes_55."
".$features_55."
".$changes_55."
".$features_54."
".$mac_changes_54."
".$features_53."
".$mac_changes_53."
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



$m549c = array(
    "num"=>"5.4.9",
    "status"=>"Recommended version (Unix command-line version)",
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

$m5213c = array(
    "num"=>"5.2.13",
    "status"=>"Older Unix command-line version",
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

$l559f = array(
    "num"=>"5.5.9",
    "status"=>"Development version (standard GUI - Fedora Core 5)",
    "file"=>"boinc_5.5.9_i686-fedora.sh",
    "date"=>"26 JULY 2006",
    "type"=>"sea",
    "features"=>"<ul>
".$features_55."
".$linux_libs_fedora_559."
".$changes_55."
    </ul>
    "
);

$l559s = array(
    "num"=>"5.5.9",
    "status"=>"Development version (standard GUI - SUSE 10.1)",
    "file"=>"boinc_5.5.9_i686-suse.sh",
    "date"=>"26 JULY 2006",
    "type"=>"sea",
    "features"=>"<ul>
".$features_55."
".$linux_libs_suse_559."
".$changes_55."
    </ul>
    "
);

$l559u = array(
    "num"=>"5.5.9",
    "status"=>"Development version (standard GUI - Ubuntu 6.06)",
    "file"=>"boinc_5.5.9_i686-ubuntu.sh",
    "date"=>"26 JULY 2006",
    "type"=>"sea",
    "features"=>"<ul>
".$features_55."
".$linux_libs_ubuntu_559."
".$changes_55."
    </ul>
    "
);

$l549 = array(
    "num"=>"5.4.9",
    "status"=>"Recommended version",
    "file"=>"boinc_5.4.9_i686-pc-linux-gnu.sh",
    "date"=>"5 May 2006",
    "type"=>"sea",
    "features"=>"<ul>
".$features_54."
".$changes_54."
    </ul>
    "
);

$l5213 = array(
    "num"=>"5.2.13",
    "status"=>"Older version",
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
        $w559,
        $w549,
        $w5213,
        $w445,
    )
);
$mac = array(
    "name"=>"Macintosh OS X (10.3 or higher)",
    "dbname" => "powerpc-apple-darwin",
    "versions"=>array(
        $m559,
        $m559c,
        $m549,
        $m549c,
        $m5213,
        $m5213c,
        $m443,
        $m443c
    )
);
$linux = array(
    "name"=>"Linux/x86",
    "dbname" => "i686-pc-linux-gnu",
    "url"=>"linux.php",
    "versions"=>array(
        $l559f,
        $l559s,
        $l559u,
        $l549,
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
