<?php
require_once("docutil.php");
page_head("BOINC Menubar");

echo "
<p>
BOINC Menubar is a front end for BOINC on Mac OS X.
It provides a simple interface,
and doesn't require any command-line typing.

<p>
BOINC Menubar does not have a main window,
application menu, or dock icon.
Instead everything is
controlled from a small icon which appears in the upper right
hand corner of your screen.
Clicking on this icon produces a menu which provides you access
to all the features of BOINC Menubar.

<p>
<img src=screenshots/menu.jpg width=261 height=412><br>
  (Figure 1)

<h3>First-time BOINC participants</h3>
<p>
If you are new to BOINC, then you need to select
some projects in which to participate.
Each project's web site lets you to create an account.
When you create an account,
you will be sent an email containing a URL and Account Key.
Collect this information and skip down to the 'Project Manager'
section for information on how to set up BOINC Menubar to run these projects.

<h3>Returning BOINC participants</h3>

<p>
If you already have a BOINC account and have run the
'boinc' command on this computer,
you will need to move existing files to a new location.
Locate the folder which served as the launch path when you ran the
'boinc' command.
It should contain such files and folders as
'client_state.xml', 'slots', and 'projects'.

<p>
Copy all these files (not the enclosing folder itself)
into '~/Library/Application Support/Boinc Data/'

<p>
Now when you run BOINC Menubar and select 'Manage Projects...'
from the status menu,
you should find a list of the projects you are currently participating in.


<h2>Managing projects</h2>
<h3>Adding Projects</h3>
<p>
Before adding projects, collect the emails containing the URLs and Account Keys for each project.

<p>
Select 'Manage Projects...' from the status menu.

<p>
<img src=screenshots/menuMouseOnManageProjects.jpg width=256 height=412><br>
  (Figure 2)
<p>
This will open up the 'Project Manager'.
<p>
    <img src=screenshots/projectManager.jpg width=499 height=262><br>
  (Figure 3)
<p>
Click the 'Add' button.
This will produce a new sheet in which you can paste in the URL and Account ID.

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
Highlight the project you wish to update, reset, or remove by clicking its name.
The three buttons at the bottom of the project manager will be enabled.
<p>
    <img src=screenshots/projectManagerButtonsEnabled.jpg width=498 height=261><br>
  (Figure 5)

<p>
Clicking 'Update' causes BOINC to contact a project's web site to obtain new preferences.
This will also report completed results and get new work if needed.
<p>
Clicking 'Reset' clears all pending work for a project.
Use this if there is a problem that is preventing your computer from working.
<p>
Clicking 'Remove' will remove the selected project.
<p>
<strong>Hint:</strong> As indicated in the project manager, double clicking a project's name or URL will take you to a projects web site.
This is particularly convenient if you want to change a project's preferences or view statistics.

<h2>The Status Menu</h2>
<p>
    <img src=screenshots/menu.jpg width=261 height=412><br>
  (Figure 6)
<h3>Controlling BOINC Using BOINC Menubar</h3>
<p>
Once BOINC Menubar has some projects to run, you can now use the menu to start and stop BOINC.
<p>
To start BOINC running, simply select 'Start' from the status menu.
BOINC will start running in the background.

<p>
If you wish to see exactly what BOINC is doing while it is running, select 'Display Log' from the status menu.
This will display a window containing all the output produced by BOINC.
<p>
    <img src=screenshots/logWindow.jpg width=685 height=332><br>
  (Figure 7)
<p>
If you want to stop BOINC running for any reason, simply select 'Stop'.
Quitting BOINC Menubar automatically stops BOINC if it is running.
<h3>Current Project and Statistics</h3>
<p>
The status menu contains four pieces of information:

<p>
The 'Current Project' is simply the project which is currently being run by BOINC.
The current project will change from time to time as BOINC stops and starts other projects.
<p>
The 'Work Completed' indicates what percentage of the current work unit has been completed.
<p>
The 'Project Credit' is the amount of credit you have received for the project which is currently running.
This is the same amount as appears on the project's web site and includes credit which you have received by running BOINC on other computers as well.
<p>
The 'Total Credit' is the amount of credit you have received for all projects which BOINC Menubar knows about.
That is, if you are participating in four projects on this computer, then the total credit will be the sum of all four project credits.

<p>
<strong>Note: </strong>The amount of credit displayed by BOINC Menubar may not be <em>exactly</em> the same as you find on project web sites.
This is because credit is only updated when a computer contacts the projects web site to send results or receive new data.
If the amount of credit differs from what you see on the web,
the amount will be updated next time the project contacts its web site.
<h2>Preferences</h2>
<p>
    <img src=screenshots/preferencesWindow.jpg width=273 height=239><br>
  (Figure 8)
<p>
Select 'Start BOINC On Application Launch' if you which BOINC to start running every time you open BOINC Menubar.
You might find this particularly useful if you want BOINC to be running all the time.

<p>
<strong>Hint:</strong> You can set BOINC Menubar to auto-launch whenever you login by adding BOINC Menubar to the login items in the 'Accounts' pane in 'System Preferences'.
Combining this with 'Start BOINC On Application Launch' will ensure that BOINC is running at all times.
<p>
Select 'Display Log On Application Launch' if you wish the Log window to open automatically every time you open BOINC Menubar.

<p>
Select Unlimited Scrollback if you would like to keep all the output produced by BOINC.
By default,
the BOINC Menubar keeps the last 10,000 lines output by BOINC.

<h2>Acknowledgements and support</h2>
<p>
BOINC Menubar was developed by
<a href=http://www.greenkeepersoftware.com>GreenKeeper Software</a>,
which has made BOINC Menubar freely available for BOINC participants.
If you wish, you can make a
<a href=http://www.greenkeepersoftware.com/donate.html>donation</a>
to GreenKeeper Software to support further development of BOINC Menubar
and future GreenKeeper Software products.
<p>
If you have questions about, problems with,
or comments concerning BOINC Menubar,
please contact GreenKeeper Software directly by email
(<a href=mailto:developer@greenkeepersoftware.com>support@greenkeepersoftware.com</a>).
If you are reporting a bug, please be as specific as possible.
<p>
Also, please be aware that some problems may be a result of BOINC
or projects running on the BOINC platform and not BOINC Menubar itself.
If you are unsure of the source of the problem,
go ahead and contact GreenKeeper Software anyway and we will do our
very best to help you or refer you to people who can.


";

page_tail();
?>
