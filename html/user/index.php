<?php
    require_once("util.inc");
    require_once("db.inc");
    require_once("project.inc");

    init_session();
    page_head("Home page");

    project_intro();

    echo "
<h3>Join ".PROJECT." </h3>
<p>
<ul>
<li><a href=create_account_form.php>Create an account</a>
</ul>

<h3>Returning participants</h3>
<ul>
<li><a href=login_form.php>Log in</a>
<li><a href=home.php>User page</a> - view stats, modify preferences
<li><a href=team.php>Teams</a> - create or join a team
<li><a href=download.php>Download BOINC</a>
<li><a href=bug_report_form.php>Report problems</a>
<li><a href=top_users.php>Top users</a>
<li><a href=top_hosts.php>Top hosts</a>
<li><a href=top_teams.php>Top teams</a>
</ul>
<!--
";


include 'schedulers.txt';

echo "-->\n";

page_tail();

?>
