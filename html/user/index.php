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
which allows you to participate in multiple
distributed computing projects at the same time.


<h3>How to join</h3>
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
include 'FILE_NAME';
?>
-->

<?php
    page_tail();
?>

