<?php
require_once("docutil.php");
page_head("Installing and running the Windows GUI client");
echo "
<p>
Install BOINC by running the installer program.
<p>
The <b>BOINC work manager</b> program controls
the use of your computer's disk, network, and processor resources.
It is normally started at boot time.
It is represented by a icon in the system tray.
Double-click on the icon to open the work manager window.
Right-click on the icon to:
<ul>
<li> <b>Suspend</b>: stop work (computation and file transfer).
<li> <b>Run based on preferences</b>: do work
    when your <a href=prefs.php>preferences</a> allow it.
<li> <b>Run always</b>: do work,
    regardless of preferences.
<li> <b>Exit</b>:  exit the work manager and all running BOINC applications.
No further work will take place until you run the work manager again.
<li> <b>Hide</b>:  close the work manager window, but do not exit.
</ul>
<p>
When the icon is flashing, 
there is an unread error message.
To view it, open the work manager window and go to the Messages tab.
<p>
The work manager window has several tabs:
<ul>
<li> <b>Projects</b>:
    Shows the projects in which this computer is participating.
    Right-click on a project name brings up a menu:
    <ul>
    <li> <b>Web site</b>: visit the project's web site.
    <li> <b>Update</b>:
        Connect to the project;
        report all completed results,
        get more work if necessary,
        and get your latest <a href=prefs.php>preferences</a>.
    <li> <b>Detach</b> from the project.
        Your computer will stop working for the project.
    <li> <b>Reset project</b>:
        Stop the project's current work, if any,
        and start from scratch.
        Use this if BOINC has become stuck for some reason.
    </ul>
<li> <b>Work</b>:
    Shows the work units currently on your computer.
    Each work unit is either
    <ul>
    <li>Downloading: input files are being downloaded.
    <li>Ready to run:
        An estimate of the total CPU time is shown.
    <li>Running: currently running.
        Elapsed CPU time and estimated percent done is shown.
    <li>Uploading: output files are being uploaded.
    <li>Ready to report: waiting to notify the scheduling server.
    </ul>
    Right-click on a work unit to:
    <ul>
    <li> <b>Show graphics</b>: open a window showing application graphics.
    </ul>
<li> <b>Transfers</b>:
    Shows file transfers (uploads and downloads).
    These may be ready to start, in progress, and completed.
<li> <b>Messages</b>:
    Shows status and error messages.
    Messages can be sorted by project or time.
    You can <a href=client_debug.php>control what messages are shown</a>.
    Messages are also written to a file 'messages.txt'.
    
<li> <b>Disk</b>:
    This shows how much disk space is available for use by BOINC,
    and how much is currently being used by each project.
</ul>
The work manager's menu items are as follows:
<ul>
<li> <b>File</b>
    <ul>
    <li><b>Run always</b>, <b>Run based on preferences</b>,
        <b>Suspend</b>: see above
    <li><b>Run Benchmarks</b>:
        run benchmark functions, which measure the speed of your processor.
        BOINC does this automatically,
        but you can repeat it whenever you want.
        The results are shown in the Messages tab.
    <li> <b>Hide</b>: close the work manager window.
        This does not exit the work manager.
        To do this, use the Exit command on the system tray icon menu.
    </ul>
<li> <b>Settings</b>
    <ul>
    <li> <b>Attach to Project</b>:
        Enroll this computer in a project.
        You must have already created an account with the project.
        You will be asked to enter the project's URL and your account ID.
    <li> <b>Proxy Server</b>
        If you connect to the web through an HTTP or SOCKS proxy,
        use this dialog to enter its address and port.
    </ul>
<li> <b>Help</b>
    <ul>
    <li> <b>About</b>: show work manage version number.
    </ul>
</ul>

<p>
Menu names and other text in the work manager can be displayed in
<a href=language.php>languages other than English</a>.
<p>
The <b>BOINC screensaver</b> can be selected using the Display Properties dialog.
The BOINC screensaver draws graphics from a running application,
if any is available.
Otherwise it draws the BOINC logo bouncing around the screen.
";
page_tail();
?>
