<?php
require_once("docutil.php");
page_head("Version history");
echo"

<h3>Version 5.8</h3>
<ul>
<li> Snooze button.
<li> CPU throttling.
<li> Simple GUI; skinnability, local prefs dialog.
<li> Preferences for limiting RAM usage.
<li> Disk usage pie chart.
<li> Support for advanced account manager features like
host-specific resource share.

<h4>Version 5.8.16</h4>
<ul>
<li> CPU model string includes Family/Model/Stepping.
<li> Fix errors if detach project while file transfers active.
<li> Limits on # of file transfers apply to uploads
and downloads separately.
<li> Add checkpoint_debug log flag.
<li> Set processor affinity mask for benchmark threads (Windows)
<li> Disable connection caching in Curl.
<li> Add 'Detach when done' flag to projects
(for clean detach, e.g. from account manager).
<li> Delete all files when detach from project.
<li> Fix bugs on Mac OS X 10.3 that could create bad users/groups.
</ul>

<h4>Version 5.8.15</h4>
<ul>
<li> Mac: eliminate AppStats helper application.
</ul>

<h4>Version 5.8.14</h4>
<ul>
<li> Win: installer fixes.
</ul>

<h4>Version 5.8.13</h4>
<ul>
<li> Fix bug where tasks are erroneously restarted after abort.
<li> Win: installer fixes.
</ul>

<h4>Version 5.8.12</h4>
<ul>
<li> Win: installer fixes.
</ul>

</ul>
<h3>Version 5.6</h3>
<ul>
<li> New CPU Scheduler.
<li> CPU capability detection.
<li> Improved security for POSIX compliant systems
(only implemented for the Mac)
<li> Support for file compression on upload.
</ul>

<h3>Version 5.4</h3>
<ul>
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
</ul>

<h3>Version 5.3</h3>
<ul>
<li> Customized clients can now be created.
<li> Account manager support has been enhanced.
<li> Farm manager support has been added.
</ul>
<h3>Version 5.2</h3>
<ul>
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
</ul>

";
page_tail();
?>
