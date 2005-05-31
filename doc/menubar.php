<?php
require_once("docutil.php");
page_head("BOINC Menubar v4.43 (4)");

echo "

<h1>Introduction</h1>
<p>
BOINC Menubar is a graphical front end for BOINC,
a software platform developed at Berkeley which allows volunteers
such as yourself to contribute unused processor cycles to help
solve problems in physics, medicine, climatology, astronomy, and more.
Combining tens of thousands of computers,
BOINC users form what is effectively the most powerful supercomputer
on the planet.

<p>
BOINC Menubar provides a simple interface for BOINC,
eliminating the need for any command-line typing.
It does not have a main window, application menu, or dock icon.
Instead, everything is controlled from a small icon which
appears in the upper right hand corner of your screen.
Clicking on this icon produces a menu which provides you
access to all the features of BOINC Menubar.

<p> <img src=screenshots/menu.jpg width=299 height=474><br>
  (Figure 1)
  
<h1>Getting Started</h1>
<h3>First-time BOINC Participants</h3>
<p>
If you are new to BOINC,
then you need to create accounts for projects
in which you would like to participate.
Please visit the main BOINC site
(<a href=http://boinc.berkeley.edu/>http://boinc.berkeley.edu/</a>)
for a list of popular projects.
<p>
When you create an account, you will be sent an email containing a URL
and Account Key.
Collect this information and skip down to the 'Project Manager'
section for information on how to set up BOINC Menubar to run these projects.

<h3>Returning BOINC Participants</h3>
<p>
If you already have a BOINC account and have run the 'boinc'
command-line on this computer, you will need to move existing files
to a new location.
Locate the folder which served as the launch path when you ran the 'boinc'
command.
It should contain such files and folders as 'client_state.xml',
'slots', and 'projects'.
    
<p>
Copy all these files (not the enclosing folder itself)
into '~/Library/Application Support/BOINC Data/'
  
<p>
Now when you run BOINC Menubar and select 'Manage Projects...' from the
status menu,
you should find a list of the projects you are currently participating in.

<h1>Managing Projects</h1>
<h3>Adding Projects</h3>
<p>
Before adding projects, collect the emails containing the URLs
and Account Keys for each project.
  
<p>
Select 'Manage Projects...' from the status menu.
  
<p>
<img src=screenshots/menuMouseOnManageProjects.jpg width=300 height=473><br>
 (Figure 2)
<p>
This will open up the 'Project Manager'.
<p>
<img src=screenshots/projectManager.jpg width=499 height=262><br> (Figure 3)
<p>
Click the 'Add' button.
This will produce a new sheet in which you can paste in a URL and Account Key.
  
<p>
<img src=screenshots/projectManagerWithAddProjectSheet.jpg width=502 height=263><br>
 (Figure 4)
<p>
After you have finished, click 'OK' and the project will be added.
<p>
<strong>Note: </strong>Please be patient as it can take a few moments for a project to be added.
  
<h3>Updating, Resetting, and Removing Projects:</h3>

<p>
Select 'Manage Projects...' from the status menu (Figure 2).
This will open up the 'Project Manager' (Figure 3).
<p>
Highlight the project you wish to update, reset, or remove
by clicking its name.
The 'Remove', 'Reset', and 'Update' buttons
at the bottom of the project manager window will be enabled.
<p>
<img src=screenshots/projectManagerButtonsEnabled.jpg width=498 height=261><br>
   (Figure 5)
  
<p>
Clicking 'Update' causes BOINC to contact a project's web site
to obtain new preferences.
This will also report completed results and get new work if needed.
<p>
Clicking 'Reset' clears all pending work for a project.
Use this if there is a problem that is preventing your computer from working.
<p>
Clicking 'Remove' will remove the selected project.
<p>
<strong>Hint:</strong> As indicated in the project manager,
double clicking a project's name or URL will take you to a projects web site.
This is particularly convenient if you want to change
a project's preferences or view statistics.
  
<h1>The Status Menu</h1>
<p>
<img src=screenshots/menu.jpg width=299 height=474><br>
  (Figure 6)
<h3>&nbsp;</h3>
<h3>Controlling BOINC Using BOINC Menubar</h3>
<p>
Once BOINC Menubar has some projects to run,
you can now use the menu to start and stop BOINC.
<p>
To start BOINC running, simply select 'Start' from the status menu.
BOINC will start running in the background.
  
<p>
If you wish to see exactly what BOINC is doing while it is running,
select 'Display Log' from the status menu.
This will display a window containing all the output produced by BOINC.
<p>
<img src=screenshots/logWindow.jpg width=685 height=332><br>
  (Figure 7)
<p>
If you want to stop BOINC running for any reason, simply select 'Stop'.
Quitting BOINC Menubar automatically stops BOINC if it is running.
<p>
You may force BOINC to run CPU benchmarks by selecting
'Run CPU Benchmarks' from the status menu.
Note that you only need to do this if you have recently modified
your computer's hardware.

<h3>Menu Statistics</h3>
<p>
The status menu contains four pieces of information:
<p>
The 'Current Project' is simply the project or projects currently
being run by BOINC.
The current project will change from time to time
as BOINC stops and starts other projects.
<p>
The 'Work Completed' indicates what percentage of the current work
unit has been completed for the current project.
<p>
The 'Project Credit' is the amount of credit you have received
for the project which is currently running.
This is the same amount as appears on the project's web site
and includes credit which you have received by running BOINC
on other computers as well.
<p>
The 'Total Credit' is the amount of credit you have received
for all projects which BOINC Menubar knows about.
That is, if you are participating in four projects on this computer,
then the total credit will be the sum of all four project credits.
  
<p>
<strong>Note: </strong>The amount of credit displayed by BOINC Menubar
may not be <em>exactly</em> the same as you find on project web sites.
This is because credit is only updated when a computer contacts
the projects web site to send results or receive new data.
If the amount of credit differs from what you see on the web,
the amount will be updated next time the project contacts its web site.

<h3>Menubar Statistics</h3>
<p>
The menubar contains two pieces of information:</p>
<p>
A small icon indicates the current project or projects. Listed below are  the most common projects and their corresponding icons.</p>
<p><img src=screenshots/CPIcon.jpg width=15 height=15> - climateprediction.net
<p><img src=screenshots/EAHIcon.jpg width=15 height=15> - Einstein@home
<p><img src=screenshots/PPAHIcon.jpg width=15 height=15> - Predictor@home
<p><img src=screenshots/SAHIcon.jpg width=15 height=15> - SETI@home
<p><img src=screenshots/BOINCIcon.jpg width=15 height=15> - Default icon for all other projects
<p>
The menubar also displays a graphical representation of the completed percentage of the current work unit as a vertical 'progress bar'.</p>

<h1>Preferences</h1>
<p> <img src=screenshots/preferencesWindow.jpg width=534 height=296><br>
  (Figure 8)

<h3>General Settings: </h3>
<p>
Select 'Start BOINC On Application Launch' if you want BOINC to
start running every time you open BOINC Menubar.
You might find this particularly useful if you want BOINC
to be running all the time.
    
<p>
<strong>Hint:</strong> You can set BOINC Menubar to auto-launch
whenever you login by adding BOINC Menubar to the login items
in the 'Accounts' pane in 'System Preferences'.
Combining this with 'Start BOINC On Application Launch'
will ensure that BOINC is running at all times.
<p>
Select 'Display Log On Application Launch' if you wish the Log window to
open automatically every time you open BOINC Menubar.
<p>
Select 'Share Data With Other Users' if you wish data to be shared
with other users running BOINC Menubar on your computer.
The data will be moved to a location where other users can access
and contribute to it.
<p>
<strong>Note:</strong> Only administrative users can set up BOINC Menubar
to share data with other users, but all users can contribute.
<p>
Select 'Unlimited Scrollback' if you would like to keep all
the output produced by BOINC.
By default, the BOINC Menubar keeps the last 10,000 lines output by BOINC.

<h3>Proxy Settings:</h3>
<p>
If you need to use a proxy, first select the type of proxy you are using:
HTTP or SOCKS.
Next, enter the address of the proxy.
If the proxy requires a username and password,
click the &quot;Set password...&quot; button.
This will produce a new sheet in which you can enter your username
and password (Figure 9).

<p><img src=screenshots/preferencesWindowWithProxyPasswordSheet.jpg width=528 height=294><br>
(Figure 9)

<p>Click &quot;OK&quot; to save

<h1>Acknowledgements and Support</h1>
<p>
BOINC Menubar was developed by
<a href=http://www.greenkeepersoftware.com>GreenKeeper Software</a>,
which has made BOINC Menubar freely available for all BOINC participants.
If you wish, you can make a
<a href=http://www.greenkeepersoftware.com/donate.html>donation</a>
to GreenKeeper Software to support further development of BOINC Menubar
and future GreenKeeper Software products.
<p>
If you have questions about, problems with, or comments concerning
BOINC Menubar, please contact GreenKeeper Software directly by
email (<a href=mailto:developer@greenkeepersoftware.com>support@greenkeepersoftware.com</a>).
If you are reporting a bug, please be as specific as possible.
<p>
Also, please be aware that some problems may be a result of BOINC or
projects running on the BOINC platform and not BOINC Menubar itself.
If you are unsure of the source of the problem,
go ahead and contact GreenKeeper Software anyway and we will do
our very best to help you or refer you to people who can.
<p>
<em><strong>Disclaimer:</strong></em> Every effort has been made on the part
of GreenKeeper Software to fully test and debug BOINC Menubar.
It should, therefore, not cause you to have any problems.
However, you are using BOINC Menubar at your own risk.
Any harm caused as a result of this software is not the fault of
GreenKeeper Software or the BOINC developers.

<h1> History </h1>
<p>
<strong>Version 4.25 (1) </strong>
<ul>
<li>First Release (Based on GreenKeeper Software's Deep Thought v1.1)</li>
</ul>
<p>
<p><strong>Version 4.25 (2)</strong></p>
<ul>
  <li>Adds support for HTTP and SOCKS proxies</li>
  <li>Adds support for  dual processor machines </li>
  <li>Fixes a bug which caused the menubar to disappear while leaving BOINC running in the background </li>
  <li>Fixes a bug which caused the application to freeze when choosing &quot;Stop&quot; or &quot;Quit BOINC Menubar&quot;</li>
  <li>Various other fixes and improvements</li>
</ul>
<p>
<p><strong>Version 4.37 (3)</strong></p>
<ul>
  <li>Includes improved BOINC client</li>
  <li>Fixes a problem which prevented some users from using proxies</li>
  <li>Now displays the current status in the menubar by changing the icon and indicating the amount of work completed</li>
  <li>Fixes a bug on dual processor machines where the status of both processes was not always being displayed </li>
  <li>Adds a preference to share data between users </li><li>Improves security by hiding and encrypting proxy password</li>
  <li>Adds ability to manually run benchmarks</li>
  <li>Improved efficiency</li>
  </ul>

<p><strong>Version 4.43 (4)</strong></p>
<ul>
  <li>Includes improved BOINC client</li></ul>

";

page_tail();
?>
