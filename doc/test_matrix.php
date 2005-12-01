<?php

require_once("docutil.php");

page_head("BOINC test cases");
echo "
<p>
For each release, please do
<ol>
<li> The general tests
<li> As many of the other test groups as possible.
</ol>

<h3>General tests</h3>

<ul>
<li>
Note fraction done of existing results.
Install the new version on top of an older version.
Make sure BOINC client and manager start up.
Make sure results resume with same fraction done.
<li>
Make sure graphics work (for all apps that support graphics).
<li>
Make sure CPU time and fraction done are increasing for running apps.
<li>Check all tabs in BOINC manager, make sure data is there.

<li> Detach from a project (e.g., alpha test) and reattach.
Make sure new work gets downloaded.

<li> Let BOINC run long enough to complete a result.
Verify that output file is uploaded,
that the result is reported,
and that new work is downloaded and started.

<li> Exit BOINC in the middle of a long file download.
Note the fraction done.
Restart BOINC.
Verify that download resumes from the same point.

<li> Suspend/resume features:
suspend and resume results,
file transfers, and projects.
<li> Test activity modes 'suspended', 'based on preferences', 'always'.

<li> Shut down your computer and reboot it.
Make sure client and manager start up OK.

</ul>

<h3>Clean install</h3>
<ul>
<li> Uninstall BOINC.
<li> Remove BOINC directory.
<li> Install new BOINC.
<li> Verify that client and manager run,
    and manager brings up the Attach Project wizard.
<li> Attach to a project (e.g. alpha)
    and make sure work is downloaded and started.
</ul>

<h3> Preferences tests</h3>
<ul>
<li> Test 'don't run when user active' preference.
<li> Test 'leave applications in memory' preference.
<li> Test scheduling period preference.
<li> Test #processors preference.
<li> Test 'use network between hours' preference.
<li> Test 'do work between hours' preference.
<li> Test venue mechanism
</ul>

</ul>
<h3>Windows single-user</h3>
<ul>
<li>Check that only the administrative and installing account can
    control the core client (via the BOINC manager)
    and view the BOINC directory.
</ul>

<h3>Windows multi-user</h3>
<ul>
<li>Everyone should be able to view the directory.  Anybody should be able to
    execute the manager but only the first person who executed the manager
    should have started a core client process.
</ul>
<h3>Windows service mode</h3>
<ul>
<li>Use of 'net start boinc' and 'net stop boinc' on the command line should
    start and stop the core client.
<li>Use of 'net pause boinc' and 'net continue boinc' should pause and resume
    the core client.
<li>The core client should be started when the machine boots up.
<li>the core client should continue processing when nobody is logged onto the
    system.
<li>Manager should be able to communicate with the core client without issue.
</ul>
<h3>Tests for modem-connected computers</h3>
<ul>
<li> Do General Tests from a modem-connected computer.
<li> Test 'confirm before connect' preference.
<li> Test 'Disconnect when done' preference.
</ul>
<h3>Tests for computers with personal firewalls</h3>
<li> Do General tests on a computer that uses a personal firewall
(ZoneAlarm, Symantec, Windows XP, etc.).
In your test report Comments field, indicate the type of personal firewall.
<h3>Tests for computers connected by HTTP proxy</h3>
<ul>
<li> Do General Tests on a computer connected via an HTTP proxy.
Use authentication if possible.
</ul>
<h3>Tests for computers connected by SOCKS proxy</h3>
<ul>
<li> Do General Tests on a computer connected via a SOCKS proxy.
Use authentication if possible.
</ul>
<h3>Tests for laptops</h3>
<ul>
<li> Test 'work while running on batteries' preference'
</ul>
<h3>Screensaver tests (Windows, Mac)</h3>
<ul>
<li> Test normal case
<li> Test change apps (set scheduling period to 1 min)
<li> Test 'no graphics capable apps'
<li> Test blank time
<li> Test power save modes
</ul>

<h3>BOINC Manager Attach Project Wizard</h3>
<ul>
<li> Test normal case, invalid URL, valid but non-BOINC URL,
    projects that are down.
<li> Test bad password, bad username
</ul>

<h3>GUI RPC authentication</h3>

<ul>

<li> Try to connect to core client
with bad password, from host not on list, etc.

</ul>
";
?>
