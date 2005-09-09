<?php
echo "
<h2>Basic tests</h2>
<p>
For each releases, alpha testers should do
<ol>
<li> The platform-independent tests
<li> As many of the other test groups as possible.
</ol>

<h3>Platform-independent tests</h3>
<p>Tests to do on all platforms.

<ul>
<li> Upgrade install:
Note fraction done of existing results.
Install BOINC on top of existing install.
Make sure results resume with same fraction done.
Make sure graphics work (for all apps that support graphics).
Make sure CPU time and fraction done are increasing for running apps.
<li>Check all tabs in BOINC manager, make sure data is there.

<li> Result completion:
Let BOINC run long enough to complete a result.
Verify that output file is uploaded,
that the result is reported,
and that new work is downloaded and started.
<li> Suspend/resume features:
suspend and resume results,
file transfers, and projects.

<li> Download restart:
Exit BOINC in the middle of a long file download.
Note the fraction done.
Restart BOINC.
Verify that download resumes from the same point.

<li> Test activity modes 'suspended', 'based on preferences', 'always'.
<li> Test 'don't run when user active' preference.
<li> Test 'leave applications in memory' preference.
<li> Test scheduling period preference.
<li> Test #processors preference.
<li> Test 'use network between hours' preference.
</ul>
<h3>Windows single-user</h3>
<ul>
<li>Check to make sure that only the administrative and installing account can
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
<h3>Mac OS X</h3>
<h3>Linux/x86 graphical</h3>
<h3>Linux/x86 command-line</h3>
<h3>Tests for modem-connected computers</h3>
<ul>
<li> Test 'confirm before connect' preference.
<li> Test 'Disconnect when done' preference.
</ul>
<h3>Tests for computers connected by HTTP proxy</h3>
<ul>
<li>Check HTTP proxy if you have one.
</ul>
<h3>Tests for computers connected by SOCKS proxy</h3>
<ul>
<li>Check SOCKS proxy if you have one.
</ul>
<h3>Tests for computers connected by HTTP proxy with authentication</h3>
<ul>
<li>Check HTTP Authentication if you use authentication. (basic, digest, NTLM, Negotiate, kerberos)
</ul>
<h3>Tests for computers connected by SOCKS proxy with authentication</h3>
<ul>
<li>Check SOCKS Authentication if you use authentication.
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

<h3>BOINC Manager wizard tests</h3>
<ul>
<li> Test normal case, invalid urls, valid urls, projects that are up, projects that are down.
<li> Test good passwords, bad password, good usernames, bad usernames.
<li> Test existing accounts, create new accounts.
<li> Test against projects that don't support usernames and passwords.
</ul>

<hr>
<h2>Advanced tests</h2>
<p>
Optional tests.

<ul>
<li> Clean install:
Uninstall BOINC.
Remove BOINC directory.
Install new BOINC.
Verify that manager starts, asks for project info.

<li> Try to connect to core client
with bad password, from host not on list, etc.
<li> (Unix) Try to overwrite executable file logged in
as different user.

<li> Test 'do work between hours' preference.
<li> Test venue mechanism
</ul>
";
?>
