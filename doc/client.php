<?
require_once("docutil.php");
page_head("Running the BOINC client");
echo "
<h3>BOINC for Windows</h3>
<p>
The <b>BOINC work manager</b> program controls
the use of your computer's disk, network, and processor resources.
It is normally started at boot time.
It is represented by a icon in the system tray.
Double-click on the icon to open the work manager window.
Right-click on the icon to:
<ul>
<li> <b>Suspend</b>: this stops current work.
<li> <b>Resume</b>: this resumes work.
<li> <b>Exit</b>:  this exits the work manager and all running BOINC applications.
No further work will take place until you run the work manager again.
</ul>
<p>
When the icon is flashing, 
there is an unread error message.
To view it, open the work manager window and go to the Messages tab.
<p>
The work manager window has several tabs:
<ul>
<li> <b>Projects</b>:
    Shows the projects in which you're participating.
    Right-click on a project name to:
    <ul>
    <li> Visit its web site.
    <li> Update preferences.
        This will connect to the project
        and get the newest version of your preferences.
        (Note: BOINC preferences are managed using the web,
        rather than in the application.
        This makes it easy to manage preferences for a number of computers.)
    <li> Detach from the project.
        Your computer will stop working for the project.
    <li> Clear project state.
        This stops the project's current work, if any,
        and starts from scratch.
        Use this if BOINC has become stuck for some reason.
    </ul>
<li> <b>Work</b>:
    Shows the work units currently on your computer.
    Each work unit is either
    <ul>
    <li>Downloading: input files are being downloaded.
    <li>Ready: waiting to run.
        An estimate of the total CPU time is shown.
    <li>In progress: currently running.
        Elapsed CPU time and estimated percent done is shown.
    <li>Uploading: output files are being uploaded.
    <li>Done: waiting to notify the scheduling server.
    </ul>
    Right-click on a work unit to:
    <ul>
    <li> Open a window showing application graphics for the work unit.
    </ul>
<li> <b>Transfers</b>:
    Shows file transfers (uploads and downloads).
    These may be ready to start, in progress, and completed.
<li> <b>Messages</b>:
    Shows status and error messages.
    Messages can be sorted by project or time.
    You can <a href=messages.html>control what messages are shown</a>.
    Messages are also written to a file 'messages.txt'.
    
<li> <b>Disk</b>:
    This shows how much disk space is available for use by BOINC,
    and how much is currently being used by each project.
</ul>
The work manager's menu items are as follows:
<ul>
<li> <b>File</b>
    <ul>
    <li> <b>Clear Messages</b>: clear the message window and file.
    <li> <b>Clear Inactive</b>: clear entries in the Transfers and Work window
        that are completed.
    <li> <b>Suspend</b>: this stops current work.
    <li> <b>Resume</b>: this resumes work.
    <li> <b>Close</b>: close the work manager window.
        This does not exit the work manager.
        To do this, use the Exit command on the system tray icon menu.
    </ul>
<li> <b>Settings</b>
    <ul>
    <li> <b>Attach to Project</b>:
        Enroll this computer in a project.
        You must have already created an account with the project.
        You will be asked to enter the project's URL and your account ID.
    <li> <b>Detach from Project</b>
        Stop using this computer for a project.
    <li> <b>Proxy Server</b>
        If you connect to the web through an HTTP or SOCKS proxy,
        enter its address and port here.
    </ul>
<li> <b>Help</b>
    <li> <b>Help</b>: show this web page.
    <li> <b>About</b>: show work manage version number.
    </ul>
<li> <b>Help</b>
</ul>

<p>
Menu names and other text in the work manager are stored in
a file called <i>language.ini</i>.
The release uses American English.
Other languages are available
<a href=http://216.198.119.31/BOINC/language_ini/language.htm>here</a>
(thanks to Robi Buechler and other volunteers for this).

<p>
The <b>BOINC screensaver</b> can be selected using the Display Properties dialog.
The BOINC screensaver draws graphics from a running application,
if any is available.
Otherwise it draws the BOINC logo bouncing around the screen.

<hr>
<h3>The BOINC command-line client</h3>
<p>
The command line client has several options:
<dl>

<dt> -attach_project
<dd> Attach this computer to a new project.
You must have an account with that project.
You will be asked for the project URL and the account ID.

<dt> -show_projects
<dd> Print a list of projects to which this computer is attached.

<dt> -detach_project
<dd> Detach this computer from a project.
You will be asked for the project URL.

<dt> -reset_project
<dd> Clear pending work for a project.
Use this if there is a problem that is preventing
your computer from working.
You will be asked for the project URL.

<dt> -update_prefs
<dd>
Contact a project's server to obtain new preferences.
You will be asked for the project URL.

<dt> -run_cpu_benchmarks
<dd>
Run CPU benchmarks.
Do this if you have modified your computer's hardware.

<dt> -help
<dd> Show client options.

<dt> -version
<dd> Show client version.
</dl>
<p>
To remove a project: quit the client.
Then delete the file 'account_PROJECT-URL.xml'
where PROJECT-URL is the project's URL.
";
page_tail();
?>
