<?php
    require_once("util.inc");
    require_once("db.inc");

    init_session();
    page_head("Home page");
?>

<h3>What we're doing</h3>
The Astropulse project, based at UC Berkeley,
uses the idle time of thousands of computers to
analyze radio telescope signals.
Astropulse uses <a href=http://boinc.berkeley.edu>BOINC</a>,
which allows you to participate in other
distributed computing projects at the same time.


<h3>How to join Astropulse</h3>
<ul>
<li>
First, <a href=create_account_form.php>create an account</a>.
You will be asked for your email address.
An <b>account key</b> will be emailed to you.
<li> If you don't already have it,
<a href=download.php>download the BOINC client</a>.
Install and run the client, and give it your account key.
<li> If you're already running the BOINC client,
select the <b>Add project</b> command
and give it your account key.
</ul>

<h3>Instructions for beta testers</h3>
When you install and run the BOINC client,
it will ask you for a project URL and account key.
Then it should download two work units,
process them, upload the results,
and continue doing this forever.
<p>
If at any point BOINC is neither computing nor transferring files,
something is probably wrong;
submit a bug report (see below).
<p>
<b>Windows users</b>:
The BOINC core client (accessable via its system tray icon)
has tabs that let you see projects, file transfers,
work in progress, and messages.
If you experience bugs, look at the file <b>stderr.txt</b>
in the BOINC directory.
If it's nonempty please include it in your bug report.
<p>
<b>Unix and Mac OS/X users</b>:
This version of the client has no GUI,
and writes to stderr and stdout.
Please include any suspicious-looking text in your bug reports.
<p>
Please report bugs by sending email to the boinc-beta
mailing list on SourceForge.net.
To join this list, go to
<a href=http://lists.sourceforge.net/lists/listinfo/boinc-beta>http://lists.sourceforge.net/lists/listinfo/boinc-beta</a>.

<h3>Returning participants</h3>
<ul>
<li><a href=login_form.php>Log in</a>
<li><a href=home.php>User page</a> - view stats, modify preferences
<li><a href=team.php>Teams</a> - create or join a team
<li><a href=bug_report_form.php>Report problems</a>
<li><a href=top_users.php>Top users</a>
<li><a href=top_hosts.php>Top hosts</a>
<li><a href=top_teams.php>Top teams</a>
</ul>

<!--
<?php
include 'schedulers.txt';
?>
-->

<?php
    page_tail();
?>

