<?php
    require_once("util.inc");
    require_once("db.inc");

    init_session();
    page_head("Home page");
?>

The Astropulse project, based at UC Berkeley,
uses the idle time of thousands of computers to
analyze radio telescope signals.


<h3>How to join Astropulse</h3>
<p>
<a href=create_account_form.php>Create an account</a>.
</ul>

<h3>Returning participants</h3>
<ul>
<li><a href=login_form.php>Log in</a>
<li><a href=home.php>User page</a> - view stats, modify preferences
<li><a href=team.php>Teams</a> - create or join a team
<li><a href=download.php>Download the BOINC client</a>
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

