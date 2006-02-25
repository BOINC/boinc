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
If you have experienced problems with BOINC that are not
exercised by any of these tests,
please post to the <a href=http://www.ssl.berkeley.edu/mailman/listinfo/boinc_alpha>boinc_alpha@ssl.berkeley.edu</a> email list.

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
Verify that output file is uploaded, that the result is reported,
and that new work is downloaded and started.


<li> Suspend/resume features:
suspend and resume results, file transfers, and projects.
<li> Test activity modes 'suspended', 'based on preferences', 'always'.

<li> Shut down your computer and reboot it.
Make sure client and manager start up OK.

</ul>

<h3>Clean install</h3>
<i>This test may cause you lose work in progress;
do it only if you know what you're doing.</i>
<ul>
<li> Remove BOINC
    (Windows: go add/remove programs control panel and uninstall BOINC).
    Then rename BOINC directory to a temporary name.
<li> Install new BOINC.
<li> Verify that client and manager run,
    and manager brings up the Attach Project wizard.
<li> Attach to the Alpha test project
    and make sure work is downloaded and started
    (don't attach to other projects,
     since any work you download will be lost).
<li> Exit BOINC, delete the BOINC directory,
    and rename temporary directory back to BOINC.
<li> Reinstall BOINC.
</ul>

<h3>File transfer restart</h3>
<ul>
<li> Exit BOINC in the middle of a long file download.
Note the fraction done.
Restart BOINC.
Verify that download resumes from the same point.
<li> Same, upload.
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
<li> Update a second project with same account email address,
and make sure preferences are propagated to the second project.
</ul>
These tests can be done by changing preferences on web site,
then Updating the project in the BOINC manager.

<h3>Windows single-user</h3>
<ul>
<li>Check that only the installing account and accounts
    with administrative privileges
    can control the core client (via the BOINC manager)
    and view the BOINC directory.
</ul>

<h3>Windows multi-user</h3>
<ul>
<li> Install BOINC in multi-user mode.
<li>
Log on as one user.
Start Manager (which starts the client).
Switch to a different user and run the Manager.
Verify that it connects the initial client
(e.g. check the messages tab and make sure start time is the same
as the original).
<li> Verify that both users can view the BOINC directory and its files.
<li>
Everyone should be able to view the directory.
Anybody should be able to execute the manager.
The first person who executed the manager
should have started the core client,
everyone else should connect to that process.
</ul>

<h3>Windows service mode</h3>
<ul>
<li> Install BOINC in service mode.
<li>Use of 'net start boinc' and 'net stop boinc' on the command line should
    start and stop the core client.
<li>Use of 'net pause boinc' and 'net continue boinc' should pause and resume
    the core client.
<li> Verify that the core client started when the machine booted up
(look at the Messages tab and look at the start time).
<li> Verify that the core client continues processing
    even when nobody is logged onto the system
    (log out and in, and check the Message tab).
<li> Verify that users other than
    admin and installing users
    can run the Manager and connect to the running core client.
</ul>
<h3>Tests for modem-connected computers</h3>
<ul>
<li> Do General Tests from a modem, ISDN, or VPN-connected computer.
<br>see the <a href=dialup.php>Dial-up Connections</a> page
for details about dial-up connections.
  <li> Test the connection options:
  <ul><li>set default connection
      <li>verify the manager uses the defaunt connection
      <li>proper notification when default hasn't been set
  </ul>
  <li> Test 'confirm before connect' preference.
     <br>'confirm' not set:
  <ul>
     <li>Test connections are automatically made when 'confirm' isn't set
     <li>Test proper operation when no user is logged on (service install)
  </ul>
     'confirm' set:
  <ul>
    <li>Check proper operation with both 'yes' and 'no' replies to the 
       confirmation dialog
    <li>Check proper operation when confirm dialog is ignored
       <br>dialog should disappear after a few minutes, with another
           prompt later
       <br>(fill in times)
    <li>With manager running only in the systray, check balloon popup
        indicating BOINC needs a network connection
  </ul>
  <li> Test 'Disconnect when done' preference.
  <li> Check that network activity is retried when dialup connection
       is established - pending scheduler requests and file transfers
       get retried.
  <li> Do these tests with both single-user and Service-mode install (Windows).
  </ul>
<h3>Tests for computers with personal firewalls</h3>
<ul>
<li> Do General tests on a computer that uses a personal firewall
(ZoneAlarm, Symantec, Windows XP, etc.).
In your test report Comments field, indicate the type of personal firewall.
</ul>
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
<li> Test 'work while running on batteries' preference.
</ul>
<h3>Screensaver tests (Windows, Mac)</h3>
<ul>
<li> Test normal case
<li> Test change apps (set scheduling period to 1 min)
<li> Test 'no graphics capable apps'
<li> Test blank time
<li> Test power save modes
<li> Test password-after-resume options
<li> Test screensaver properties Preview function
<li> (Windows only?) Test multi-user - switch to different user and 
run screensaver
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
