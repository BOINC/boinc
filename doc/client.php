<?
require_once("docutil.php");
page_head("Installing and running the BOINC client");
echo "
<ul>
<li><a href=#win>Instructions for the Windows client</a>
<li><a href=#cmdline>Instructions for the command-line client</a>
(Mac OS/X, Linus, Unix)
<li><a href=#mac>Additional instructions for Mac OS/X</a>
</ul>
<a name=win>
<h3>BOINC for Windows</h3>
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
<a name=cmdline>
<h3>The BOINC command-line client</h3>
<p>
Install the BOINC client by using gunzip to decompress the application.
Use 'chmod' to make it executable.
Put it in a directory by itself.
Run it manually, from your login script,
or from system startup files.
<p>
The command line client has several options:
";
list_start();
list_item("-attach_project",
    "Attach this computer to a new project.
    You must have an account with that project.
    You will be asked for the project URL and the account ID."
);
list_item("-show_projects",
    "Print a list of projects to which this computer is attached."
);

list_item("-detach_project URL",
    "Detach this computer from a project."
);

list_item("-reset_project URL",
    "Clear pending work for a project.
    Use this if there is a problem that is preventing
    your computer from working."
);

list_item("-update_prefs URL",
    "Contact a project's server to obtain new preferences."
);

list_item("-run_cpu_benchmarks",
    "Run CPU benchmarks.
    Do this if you have modified your computer's hardware."
);
list_item("-help",
    "Show client options."
);

list_item("-version",
    "Show client version."
);
list_end();
echo"
<a name=mac>
<h3>Installing BOINC on Mac OS/X</h3>
<p>
The Mac OS X client will unpack correctly with gunzip on Mac OS X
10.2 (jaguar) or 10.3 (panther) as long as you type the command
within Terminal. Stuffit 7.x or newer will work under the Finder
in either OS X or OS 9, but I'd recommend using 'gunzip' or 'gzip -d'
within Terminal instead.

<p>
However, the two main browsers on OS X (IE 5.2.x and Safari 1.x) will
automatically unpack downloads by default, so your work may already
be done.

<p>
If you use IE, the boinc client will download and automatically unpack
leaving two files:
<ol>
<li>
   boinc_2.12_powerpc-apple-darwin
     [this will have the stuffit icon in the finder]

<li>
   boinc_2.12_powerpc-apple-darwin7.0.0
     [this will not have any icon in the finder]

</ol>
<p>
 #2 is the unpacked program ready-to-run. You can just start Terminal
 and run boinc.

<p>
If you use Safari, the boinc client will download and automatically
unpack, leaving a single file:
<ul>
<li>
   boinc_2.12_powerpc-apple-darwin7.0.0
     [this will not have any icon in the finder]
</ul>
 This is the unpacked program, but it's not yet ready-to-run (this is
 a bug with how Safari handles gzipped downloads; we'll fix this soon).

 <p>
 Here's what you have to do to fix the Safari download (apologies if
 you already know how to do this):

 <ul>
   <li> Create a folder in your home directory and put the boinc
     file in it
   <li> Start Terminal
   <li> 'cd' to the folder you just created
   <li> Type 'chmod +x boinc_2.12_powerpc-apple-darwin7.0.0'
     (without the quotes)
</ul>
 Now you can run BOINC.
 ";
page_tail();
?>
