<?php
    require_once("util.inc");
    require_once("db.inc");
    require_once("project_specific/project.inc");

    init_session();
    page_head("Home page");

    project_intro();

    echo "
<h3>Join ".PROJECT." </h3>
<p>
<ul>
<li><a href=".URL_BASE."/info.php>Rules and policies <b>[read this first]</b></a>
<li><a href=".URL_BASE."/create_account_form.php>Create an account</a>
</ul>

<h3>Returning participants</h3>
<ul>
<li><a href=".URL_BASE."/login_form.php>Log in</a>
<li><a href=".URL_BASE."/home.php>User page</a> - view stats, modify preferences
<li><a href=".URL_BASE."/team.php>Teams</a> - create or join a team
<li><a href=".URL_BASE."/download.php>Download BOINC</a>
<li><a href=".URL_BASE."/bug_report_form.php>Report problems</a>
</ul>
<h3>Community</h3>
<ul>
<li><a href=".URL_BASE."/profile_menu.php>User profiles</a>
</ul>
<h3>Project totals and leader boards</h3>
<ul>
<li><a href=".URL_BASE."/totals.php>Project totals</a>
<li><a href=".URL_BASE."/top_users.php>Top users</a>
<li><a href=".URL_BASE."/top_hosts.php>Top computers</a>
<li><a href=".URL_BASE."/top_teams.php>Top teams</a>
</ul>
<!--
";

include 'schedulers.txt';

echo "-->\n";

page_tail();

?>
