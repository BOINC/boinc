<?php
require_once("docutil.php");

page_head("Release notes");

echo "
<ul>
    <li> <a href=#new>What's new in 5.4</a>
    <li> <a href=#install>Installing</a>
    <li> <a href=#uninstall>Uninstalling</a>
    <li> <a href=#bugs>Known issues</a>
    <li> <a href=#troubleshoot>Troubleshooting</a>
</ul>

<h2>What's new in 5.4</h2>
<ul>
<li> Support for 'Account managers' - web sites
that let you browse BOINC projects, attach/detach,
change resource share and settings,
all with point-and-click simplicity.
Account managers are also great if you have several computers -
you just have to make changes once.
<li>
General preferences can be overridden by a local file;
details are <a href=http://boinc.berkeley.edu/prefs_override.php>here</a>.
<li>
BOINC now alerts you whenever it needs you
to create a network connection.
</ul>

We recommend that all BOINC users upgrade to 5.4.

<p>
A detailed revision history is
<a href=rev_history.php>here</a>.

<a name=install></a>
<h2>Installing</h2>

<h3>Windows</h3>
BOINC can be installed in any of several modes:

<ul>
<li>
<b>Single-user installation</b>
<p>
This is the recommended mode.
BOINC will run while you (the installing user) are logged in.
BOINC is listed in the Start menu of you,
but not other users.
The 'Show graphics' command in the BOINC manager
works only for you.
The BOINC screensaver shows application
graphics only for you
(other users can run the screensaver but will see textual information only).

<li>
<b>Shared installation</b>
<p>
BOINC runs whenever any user is logged in.
BOINC is listed in the Start menu of all users.
While BOINC is running, it runs as a particular user
(either the first user to log in, or the first to run BOINC).
The 'Show graphics' command in the BOINC manager works only for this user.
The BOINC screensaver shows application graphics only for this user
(other users can run the screensaver but will see textual information only).

<li>
<b>Service installation</b>
<p>
BOINC runs all the time (even when no one is logged in).
BOINC is listed in the Start menu of the installing user, but not other users.  
The 'Show graphics' command in the BOINC manager will not work for any user.
The BOINC screensaver will only show textual information.
</ul>

<h3>Linux</h3>
BOINC for Linux is distributed as a self-extracting archive.
This type of installation requires that you be familiar with the
UNIX command-line interface.
<p>
The download files have names like
<code>boinc_5.2.13_i686-pc-linux-gnu.sh</code>.
After downloading the file, type
<pre>
sh boinc_5.2.13_i686-pc-linux-gnu.sh
</pre>
This will create a directory BOINC/ with the following files:
<dl>
<dt> boinc
<dd> The BOINC core client
<dt> boincmgr
<dd> The BOINC manager
<dt>
run_client
<dd> A script that cd's into the BOINC directory and runs the core client.
<dt>
run_manager
<dd> A script that cd's into the BOINC directory and runs the manager.
</dl>

<p>
You may want to
<a href=auto_start.php>automatically start the core client</a>
at boot time.

<a name=uninstall></a>
<h2>Uninstalling</h2>
<h3>Windows</h3>
In the Start menu, select Programs / BOINC / Uninstall.
Or in the Start menu, select Settings / Add or remove programs.

<a name=issues></a>
<h2>Known issues</h2>
<ul>
<li> If you use a proxy server, please hold off upgrading for now.
We have a fix in the works for proxies that use NTLM authentication.
</ul>
<h3>Windows</h3>
<ul>
<li>
if BOINC applications are repeatedly crashing on your computer,
it's possibly that you need to
<a href=directx.php>upgrade to the latest version of DirectX</a>.
<li> If BOINC runs at the same time as Windows XP 3-D screensavers,
the system becomes sluggish and unresponsive.
<li> Applications that were built before October 2004
do not display screensaver graphics with the Service or Shared install type,
or the Single-user install type with
the password protect screensaver option on NT based machines.
</ul>

<a name=troubleshoot></a>
<h2>Troubleshooting</h2>
If you have problems with BOINC,
here are some steps you can take:
<ul>
<li> If the problem is with a particular project,
go to the 'Questions and Answers' area of the project's web site.
You may find that there's already a solution to your problem.
If not, post it there, and you'll get help from other users.
<li> If project-specific problems persist,
use the BOINC Manager to 'reset' that project.
This will erase any jobs in progress and start from scratch.
<li> If you have problems with BOINC itself,
get help from the <a href=dev/>BOINC message boards</a>.
</ul>

";
page_tail();
?>
