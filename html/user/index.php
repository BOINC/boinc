<?php
    require_once("db.inc");
    require_once("util.inc");
    require_once("project_specific/project.inc");

    // don't want to use DB here, because master page won't be visible
    // if DB is down
    //
    //db_init();
    //$user = get_logged_in_user(false);
    //page_head("Home page", $user);

    page_head(PROJECT);

    project_intro();

    echo "
<br><br>
<table cellpadding=8>
<tr><td width=40%>
<h3>Join ".PROJECT." </h3>
<p>
<ul>
<li><a href=".URL_BASE."info.php>Rules and policies <b>[read this first]</b></a>
<li><a href=".URL_BASE."create_account_form.php>Create account</a>
</ul>

<h3>Returning participants</h3>
<ul>
<li><a href=".URL_BASE."login_form.php>Log in</a>
<li><a href=".URL_BASE."home.php>User page</a> - view stats, modify preferences
<li><a href=".URL_BASE."team.php>Teams</a> - create or join a team
<li><a href=".URL_BASE."download.php>Download BOINC</a>
</ul>
<h3>Community</h3>
<ul>
<li><a href=".URL_BASE."profile_menu.php>User profiles</a>
<li><a href=".URL_BASE."forum/>Forum</a>
<li><a href=".URL_BASE."forum/help_desk.php>Help Desk / FAQ</a>
</ul>
<h3><a href=stats.php>Project totals and leader boards</a></h3>
<!--  Deprecated
<ul>
<li><a href=".URL_BASE."totals.php>Project totals</a>
<li><a href=".URL_BASE."top_users.php>Top users</a>
<li><a href=".URL_BASE."top_hosts.php>Top computers</a>
<li><a href=".URL_BASE."top_teams.php>Top teams</a>
<li><a href=".URL_BASE."stats.php>XML statistics data</a>
</ul>
-->
</td>
<td valign=top bgcolor=dddddd>
<center>
<h3>News</h3>
</center>
";
project_news();
echo "
</td>
</tr></table>
<!--
";

include 'schedulers.txt';

echo "-->\n";

page_tail();

?>
