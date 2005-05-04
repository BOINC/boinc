<?php
require_once("docutil.php");
page_head("The BOINC manager");
echo "
<p>
The <b>BOINC manager</b> program controls
the use of your computer's disk, network, and processor resources.
It is normally started at boot time.
It is represented by a icon in the system tray.
</p>

<img src=mgrsystraymenu.png>

<p>
Double-click on the icon to open the BOINC manager window.
Right-click on the icon to:
<ul>
<li> <b>Open BOINC Manager</b>: opens the current BOINC Manager.
<li> <b>Run always</b>: do work, regardless of preferences.
<li> <b>Run based on preferences</b>: do work
    when your <a href=prefs.php>preferences</a> allow it.
<li> <b>Suspend</b>: stop work (computation and file transfer).
<li> <b>Disable BOINC network access</b>:  setting this keeps BOINC
from attempting to contact any of the project servers.  It is useful
for those on dial-up connections who do not want to be bothered with
BOINC prompting to connect or disconnect for a time.
<li> <b>About BOINC Manager</b>:  displays useful information about the
BOINC Manager.
<li> <b>Exit</b>:  exit the BOINC manager and all running BOINC applications.
No further work will take place until you run the BOINC manager again.
</ul>
</p>
<img src=mgrsystrayballoon.png>
<p>
Hovering over the BOINC icon will display a status balloon which contains
the project it is currently working on, how far along it is, and which
computer it is connected too.
</p>
<h1>BOINC Manager Tabs</h1>
<h2>Projects</h2>
<p>Shows the projects in which this computer is participating.</p>
<img src=mgrprojects.png>

<ul>
<li>Suspended: 
    The project is currently suspended.
<li>Retry in ...:
    The client will wait the specified amount of time before attempting
    to contact the project server again.
<li>Won't get new work:
    The project will not fill the cache for this project 
    when it runs out of work.
</ul>

<p>Click on a project name to enable the following additional buttons:</p>
<ul>
<li> <b>Allow new work</b>:
    Allow the project to download additional work, if needed.
<li> <b>Attach to new project</b>:
    Attach to a new project with the account key and URL sent to you
    by the project administrator.
<li> <b>Detach</b>:
    Your computer will stop working for the project.
<li> <b>No new work</b>:
    Do not download any additional work for this project.
<li> <b>Reset project</b>:
    Stop the project's current work, if any,
    and start from scratch.
    Use this if BOINC has become stuck for some reason.
<li> <b>Resume</b>:
    Resumes processing of a previous suspended project.
<li> <b>Suspend</b>:
    Suspends any further processing of this project.
<li> <b>Update</b>:
    Connect to the project;
    report all completed results,
    get more work if necessary,
    and get your latest <a href=prefs.php>preferences</a>.
</ul>

<p>Project administrators can add <a href=gui_urls.php>buttons</a> 
   to the manager to quickly navigate the project website.</p>

<h2>Work</h2>
<p>Shows the work units currently on your computer.
    Each work unit is either</p>
<img src=mgrwork.png>

<ul>
<li>Aborted: 
    Result has been aborted and will be reported to the project server
    as a computational error.
<li>Downloading: 
    Input files are being downloaded.
<li>Paused: 
    Result has been suspended by the client-side scheduler and will be
    resumed the next time the project comes up in the processing rotation.
<li>Ready to report: 
    Waiting to notify the scheduling server.
<li>Ready to run:
    An estimate of the total CPU time is shown.
<li>Running:
    Elapsed CPU time and estimated percent done is shown.
<li>Suspended: 
    Result has been suspended.
<li>Uploading: 
    Output files are being uploaded.
</ul>

<p>Click on a result name to enable the following additional buttons:</p>
<ul>
<li> <b>Abort</b>: 
    Abort processing for a result. NOTE: This will prevent you from receiving
    credit for any work already completed.
<li> <b>Resume</b>:
    Resumes processing of a previous suspended result.
<li> <b>Show graphics</b>: 
    Open a window showing application graphics.
<li> <b>Suspend</b>:
    Suspends any further processing of this result.
</ul>

<h2>Transfers</h2>
<p>Shows file transfers (uploads and downloads).
    These may be ready to start, in progress, and completed.</p>
<img src=mgrtransfers.png>

<ul>
<li>Aborted: 
    Result has been aborted and will be reported to the project server
    as a computational error.
<li>Downloading: 
    Input files are being downloaded.
<li>Retry in ...:
    The client will wait the specified amount of time before attempting
    to contact the project server again.
<li>Uploading: 
    Output files are being uploaded.
</ul>

<p>Click on a file name to enable the following additional buttons:</p>
<ul>
<li> <b>Retry Now</b>: 
    Retry the file transfer now.
<li> <b>Abort Transfer</b>:
    Abort the file transfer. NOTE: This will prevent you from receiving credit
    for any work already completed.
</ul>

<h2>Messages</h2>
<p>Shows status and error messages.
    Messages can be sorted by project or time.
    You can <a href=client_debug.php>control what messages are shown</a>.
    Messages are also written to a file 'stdoutdae.txt'.</p>
<img src=mgrmessages.png>

<p>Click on one or more messages to enable the following additional buttons:</p>
<ul>
<li> <b>Copy all messages</b>: 
    Copies all the messages to the clipboard.
<li> <b>Copy selected messages</b>:
    Copies the highlighted messages to the clipboard. NOTE: To highlight a message
    hold down the CTRL key and then click on the messages you want to store in the
    clipboard.  When done click on the 'Copy selected messages' button to copy them
    to the clipboard.
</ul>

<h2>Statistics</h2>
<p>Shows some simple charts and graphs about the user and host progress</p>
<img src=mgrstatistics.png>
<p>NOTE: This feature requires three connections to each project scheduler on three
   different days before it starts to work properly.</p>
<p>Click on any of the buttons to change to a different chart:</p>
<ul>
<li> <b>Show user total</b>: 
    Shows the user's credit totals for each project.
<li> <b>Show user average</b>:
    Shows the users's credit averages for each project.
<li> <b>Show host total</b>: 
    Shows the host's credit totals for each project.
<li> <b>Show host average</b>:
    Shows the host's credit averages for each project.
</ul>

<h2>Disk</h2>
<p>This shows how much disk space is currently being used by each project.</p>
<img src=mgrdisk.png>

<h1>BOINC Manager Menus</h1>

The BOINC manager's menu items are as follows:
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
    <li> <b>Hide</b>: close the BOINC manager window.
        This does not exit the BOINC manager.
        To do this, use the Exit command on the system tray icon menu.
    </ul>
<li> <b>Settings</b>
    <ul>
    <li> <b>Attach to Project</b>:
        Enroll this computer in a project.
        You must have already created an account with the project.
        You will be asked to enter the project's URL and your account key.
    <li> <b>Proxy Server</b>
        If you connect to the web through an HTTP or SOCKS proxy,
        use this dialog to enter its address and port.
    </ul>
<li> <b>Help</b>
    <ul>
    <li> <b>About</b>: show BOINC manage version number.
    </ul>
</ul>

<p>
Menu names and other text in the BOINC manager can be displayed in
<a href=language.php>languages other than English</a>.
<p>
On Windows, the <b>BOINC screensaver</b>
can be selected using the Display Properties dialog.
The BOINC screensaver draws graphics from a running application,
if any is available.
Otherwise it draws the BOINC logo bouncing around the screen.
";
page_tail();
?>
