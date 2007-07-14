<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/news.inc");
require_once("../inc/cache.inc");
require_once("../inc/uotd.inc");
require_once("../inc/sanitize_html.inc");
require_once("../inc/translation.inc");
require_once("../inc/text_transform.inc");
require_once("../project/project.inc");
require_once("../project/project_news.inc");


function show_nav() {
    $config = get_config();
    $master_url = parse_config($config, "<master_url>");
    echo "<div id=\"mainnav\">
        <h2>About ".PROJECT."</h2>
        XXX is a research project that uses Internet-connected
        computers to do research in XXX.
        You can participate by downloading and running a free program
        on your computer.
        <p>
        XXX is based at 
        [describe your institution, with link to web page]
        <ul>
        <li> [Link to page describing your research in detail]
        <li> [Link to page listing project personnel, and an email address]
        </ul>
        <h2>Join ".PROJECT."</h2>
        <ul>
        <li><a href=\"info.php\">".tr(RULES_TITLE)."</a>
        <li> This project uses BOINC.
            If you're already running BOINC, select Attach to Project.
            If not, <a target=\"_new\" href=\"http://boinc.berkeley.edu/download.php\">download BOINC</a>.
        <li> When prompted, enter <br><b>".$master_url."</b>
        <li> If you're running a command-line or pre-5.0 version of BOINC,
            <a href=create_account_form.php>create an account</a> first.
        <li> If you have any problems,
            <a href=http://boinc.berkeley.edu/help.php>get help here</a>.
        </ul>

        <h2>Returning participants</h2>
        <ul>
        <li><a href=\"home.php\">Your account</a> - view stats, modify preferences
        <li><a href=\"team.php\">Teams</a> - create or join a team
        <li><a href=\"cert1.php\">Certificate</a>
        <li> <a href=\"apps.php\">".tr(APPS_TITLE)."</a>
        </ul>
        <h2>Community</h2>
        <ul>
        <li><a href=\"".URL_BASE."/profile_menu.php\">Participant profiles</a>
        <li><a href=\"forum_index.php\">Message boards</a>
        <li><a href=\"forum_help_desk.php\">Questions and answers</a>
        <li><a href=\"stats.php\">Statistics</a>
        </ul>
        </div>
    ";
}

$caching = false;

if ($caching) {
    start_cache(INDEX_PAGE_TTL);
}

$stopped = web_stopped();
$rssname = PROJECT . " RSS 2.0" ;
$rsslink = URL_BASE . "/rss_main.php";

if (defined("CHARSET")) {
    header("Content-type: text/html; charset=".tr(CHARSET));
}

echo "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd\">";
echo "<html>
    <head>
    <title>".PROJECT."</title>
    <link rel=\"stylesheet\" type=\"text/css\" href=\"white.css\">
    <link rel=\"alternate\" type=\"application/rss+xml\" title=\"".$rssname."\" href=\"".$rsslink."\">
    </head><body>
    <h1>".PROJECT."</h1>
    <table cellpadding=\"8\" cellspacing=\"4\">
    <tr><td rowspan=\"2\" valign=\"top\" width=\"40%\">
";

if ($stopped) {
    echo "
        <b>".PROJECT." is temporarily shut down for maintenance.
        Please try again later</b>.
    ";
} else {
    db_init();
    show_nav();
}

echo "
    <p>
    Powered by <a href=\"http://boinc.berkeley.edu/\"><img align=\"middle\" border=\"0\" src=\"http://boinc.berkeley.edu/boinc_logo_trans.gif\" alt=\"BOINC Logo\"></a>
    </td>
";

if (!$stopped) {
    $profile = get_current_uotd();
    if ($profile) {
        echo "
            <td id=\"uotd\">
            <h2>User of the day</h2>
        ";
        $user = lookup_user_id($profile->userid);
        echo uotd_thumbnail($profile, $user);
        echo user_links($user)."<br>";
        echo sub_sentence(output_transform(strip_tags($profile->response1)), ' ', 150, true);
        echo "</td></tr>\n";
    }
}

echo "
    <tr><td id=\"news\">
    <h2>News</h2>
    <p>
";
show_news($project_news, 5);
if (count($project_news) > 5) {
    echo "<a href=\"old_news.php\">...more</a>";
}
echo "
    <p class=\"smalltext\">
    News is available as an
    <a href=\"rss_main.php\">RSS feed</a> <img src=img/xml.gif alt=\"XML\">.</p>
    </td>
    </tr></table>
";

include 'schedulers.txt';

echo "
";

if ($caching) {
    page_tail_main(true);
    end_cache(INDEX_PAGE_TTL);
} else {
    page_tail_main();
}

?>
