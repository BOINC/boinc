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
<tr><td valign=top width=40%>
";
if (project_is_stopped()) {
    echo "
        <b>".PROJECT." is temporarily shut down for maintenance.
        Please try again later.
    ";
} else {
    echo "
        <h3>Join ".PROJECT." </h3>
        <p>
        <ul>
        <li><a href=".URL_BASE."info.php>Rules and policies <b>[read this first]</b></a>
        <li><a href=intro.php>Getting started</a>
        <li><a href=".URL_BASE."create_account_form.php>Create account</a>
        <li><a href=apps.php>Applications</a>
        <li><a href=debug.php>Download debugging files</a>
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
        <li><a href=".URL_BASE."forum/>Message boards</a>
        <li><a href=".URL_BASE."forum/help_desk.php>Questions and problems</a>
        </ul>
        <h3><a href=stats.php>Project totals and leader boards</a></h3>
        <ul>
        <li><a href=top_users.php>Top users</a>
        <li><a href=top_hosts.php>Top hosts</a>
        <li><a href=top_teams.php>Top teams</a>
        </ul>
        <p>
        <h3>Powered by <a href=http://boinc.berkeley.edu><img align=middle border=0 src=http://boinc.berkeley.edu/boinc_logo_trans.gif></a>
        </h3>
    ";
}
echo "
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

page_tail(true);

?>
