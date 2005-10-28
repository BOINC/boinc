<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/news.inc");
require_once("../inc/cache.inc");
require_once("../inc/uotd.inc");
require_once("../inc/sanitize_html.inc");
require_once("../project/project.inc");
require_once("../project/project_news.inc");


function show_nav() {
    echo "
        <h3>Join ".PROJECT." </h3>
        <p>
        <ul>
        <li><a href=info.php>Rules and policies <b>[read this first]</b></a>
        <li><a href=http://boinc.berkeley.edu/intro_user.php>Getting started</a>
        <li><a href=apps.php>Applications</a>
        </ul>

        <h3>Returning participants</h3>
        <ul>
        <li><a href=home.php>Your account</a> - view stats, modify preferences
        <li><a href=team.php>Teams</a> - create or join a team
        <li><a href=http://boinc.berkeley.edu/download.php>Download BOINC</a>
        <li><a href=download_network.php>Add-ons</a>
        </ul>
        <h3>Community</h3>
        <ul>
        <li><a href=".URL_BASE."profile_menu.php>Participant profiles</a>
        <li><a href=forum_index.php>Message boards</a>
        <li><a href=forum_help_desk.php>Questions and answers</a>
        </ul>
        <h3>Project totals and leader boards</h3>
        <ul>
        <li><a href=top_users.php>Top participants</a>
        <li><a href=top_hosts.php>Top computers</a>
        <li><a href=top_teams.php>Top teams</a>
        <li><a href=stats.php>Other statistics</a></h3>
        </ul>
    ";
}

$caching = false;

if ($caching) {
    start_cache(INDEX_PAGE_TTL);
}

$stopped = project_is_stopped();
$rssname = PROJECT . " RSS 2.0" ;
$rsslink = URL_BASE . "rss_main.php";

echo "
    <head>
    <title>".PROJECT."</title>
    <link rel=stylesheet type=text/css href=white.css>
    <link rel=alternate type=text/xml title=\"$rssname\" href=\"$rsslink\" />
    </head><body>
    <h1>".PROJECT."</h1>
    <table cellpadding=8 cellspacing=4>
    <tr><td rowspan=2 valign=top width=40%>
";

if ($stopped) {
    echo "
        <b>".PROJECT." is temporarily shut down for maintenance.
        Please try again later.
    ";
} else {
    db_init();
    show_nav();
}

echo"
    <p>
    Powered by <a href=http://boinc.berkeley.edu><img align=middle border=0 src=http://boinc.berkeley.edu/boinc_logo_trans.gif></a>
    </td>
";

if (!$stopped) {
    $profile = get_current_uotd();
    if ($profile) {
        echo "
            <td valign=top bgcolor=f4eeff>
            <b>User of the day</b><br><br>
        ";
        $user = lookup_user_id($profile->userid);
        echo uotd_thumbnail($profile, $user);
        echo user_links($user)."<br>";
        echo sub_sentence(strip_tags($profile->response1), ' ', 150, true);
        echo "</td></tr>\n";
    }
}

echo "
    <tr><td valign=top bgcolor=dddddd>
    <b>News</b>
    <p>
";
show_news($project_news, 5);
if (count($project_news > 5)) {
        echo "<a href=old_news.php>...more</a>\n";
}
echo "
    <p>
    <font size=-2>News is available as an
    <a href=rss_main.php>RSS feed</a>.</font>
    </td>
    </tr></table>
<font color=ffffff>
";

include 'schedulers.txt';

echo "</font>\n";

if ($caching) {
    page_tail_main(true);
    end_cache(INDEX_PAGE_TTL);
} else {
    page_tail_main();
}

?>
