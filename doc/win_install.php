<?php

require_once("docutil.php");

page_head("Windows installation options");
echo "

BOINC's Windows installer installs several programs:

<ul>
<li> <b>core client</b>: the program that manages file transfers
and execution of applications.
<li> <b>manager</b>: the GUI to the core client.
<li> <b>screensaver</b>: a program that runs when the machine is idle.
Typically it sends a message to the core client,
telling it to do screensaver graphics.
</ul>

BOINC can be installed in any of several 'modes':

<h2>Single-user mode</h2>
<p>
This is the default.
The goals are simplicity and nice graphics.
<p>
Say the install is done by user X.
The manager runs automatically when X logs in.
The manager starts up the core client.
The core client it runs as a regular process, not a service.
If the manager crashes the core client continues to run.
The user can re-run the manager.
When the user logs out, the manager, the core client,
and any running applications exit.
<p>
Files (in the BOINC directory) are owned by user X.
<p>
Detection of mouse/keyboard is done by the manager.
<p>
The screensaver works as it currently does,
except that we'll pass window-station/desktop info
so that the password-protected screensaver mechanism will work.
<p>
Other users can't run the BOINC manager.

<h2>Shared mode</h2>
<p>
This is the same as single-user mode except
that the BOINC manager (and core client)
run whenever any user is logged in.
Processes run as whoever is logged in.

<p>
If someone logs in while BOINC is already running,
it will not start a new instance of BOINC.


<h2>Service mode</h2>
<p>
This is for situations, such as a PC lab in a school,
where the administrator wants BOINC to run on the machine
all the time (even when no one is logged in)
but doesn't want any other users to be able to see or control BOINC.
<p>
The core client runs as a service, started at boot time.
On Windows 2003 and greater is runs under the 'network service' account.
Otherwise it runs as the installing user.
<p>
The manager checks mouse/keyboard input
and conveys idle state to the core client.
There is no screensaver capability.
Only the installing user can run the BOINC manager.
Files are accessable only to the installing user.

<h2>Service graphical mode</h2>
<p>
<b>NOTE: this mode is not implemented and may never be,
because of technical difficulties
and the undesirability of running BOINC under the 'local system' account.</b>
<p>
This is for PCs that have multiple users,
all of whom want to see graphics and have control over BOINC.
BOINC should run when no one is logged in.

<p>
The core client runs as a service, started at boot time.
It runs under the 'local system' account
(and hence so do all applications).
The manager starts at login for all users.
The manager checks mouse/keyboard input
and conveys idle state to the core client.

<p>
The screensaver either does graphics itself
(based on info obtained from the core client via RPC)
or (via the core client) has an application do the graphics.
In this case the application must switch to the same
window station and desktop as the screensaver.


<h2>Customizing the installer</h2>
<p>
The new BOINC installer is an MSI package.
Suppose you want to modify it so that you can
deploy BOINC across a Windows network using Active Directories,
and have all the PCs attached to a particular account.
Here's how to do this:
<ul>
<li> Using ORCA, edit the installer to set the installation
parameters to what you want.
<li> The global property ACCOUNTS_LOCATION specifies
(either in UNC or drive:path format)
a directory containing initial account files (normally null).
You can edit this to point to the account file you want.
For large-scale deployments it is probably safer
to use UNC paths.
</ul>

";
?>
