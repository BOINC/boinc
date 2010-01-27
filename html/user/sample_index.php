<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/news.inc");
require_once("../inc/cache.inc");
require_once("../inc/uotd.inc");
require_once("../inc/sanitize_html.inc");
require_once("../inc/translation.inc");
require_once("../inc/text_transform.inc");
require_once("../project/project.inc");

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
        <li><a href=\"info.php\">".tra("Read our rules and policies")."</a>
        <li> This project uses BOINC.
            If you're already running BOINC, select Attach to Project.
            If not, <a target=\"_new\" href=\"http://boinc.berkeley.edu/download.php\">download BOINC</a>.
        <li> When prompted, enter <br><b>".$master_url."</b>
        <li> If you're running a command-line or pre-5.0 version of BOINC,
            <a href=\"create_account_form.php\">create an account</a> first.
        <li> If you have any problems,
            <a target=\"_new\" href=\"http://boinc.berkeley.edu/help.php\">get help here</a>.
        </ul>

        <h2>Returning participants</h2>
        <ul>
        <li><a href=\"home.php\">Your account</a> - view stats, modify preferences
        <li><a href=\"team.php\">Teams</a> - create or join a team
        <li><a href=\"cert1.php\">Certificate</a>
        <li> <a href=\"apps.php\">".tra("Applications")."</a>

        </ul>
        <h2>".tra("Community")."</h2>
        <ul>
        <li><a href=\"profile_menu.php\">".tra("Profiles")."</a>
        <li><a href=\"user_search.php\">User search</a>
        <li><a href=\"forum_index.php\">".tra("Message boards")."</a>
        <li><a href=\"forum_help_desk.php\">".tra("Questions and Answers")."</a>
        <li><a href=\"stats.php\">Statistics</a>
        <li><a href=language_select.php>Languages</a>
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
$rsslink = URL_BASE . "rss_main.php";

header("Content-type: text/html; charset=utf-8");

echo "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">";

echo "<html>
    <head>
    <title>".PROJECT."</title>
	<link rel=\"stylesheet\" type=\"text/css\" href=\"main.css\" media=\"all\" />
    <link rel=\"stylesheet\" type=\"text/css\" href=\"".STYLESHEET."\">
    <link rel=\"alternate\" type=\"application/rss+xml\" title=\"".$rssname."\" href=\"".$rsslink."\">
";
include 'schedulers.txt';
echo "
    </head><body>
    <span class=page_title>".PROJECT."</span>
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
    <a href=\"http://boinc.berkeley.edu/\"><img align=\"middle\" border=\"0\" src=\"img/pb_boinc.gif\" alt=\"Powered by BOINC\"></a>
    </p>
    </td>
";

if (!$stopped) {
    $profile = get_current_uotd();
    if ($profile) {
        echo "
            <td id=\"uotd\">
            <h2>".tra("User of the day")."</h2>
        ";
        show_uotd($profile);
        echo "</td></tr>\n";
    }
}

echo "
    <tr><td id=\"news\">
    <h2>News</h2>
    <p>
";
include("motd.php");
show_news(0, 5);
echo "
    </td>
    </tr></table>
";


if ($caching) {
    page_tail_main(true);
    end_cache(INDEX_PAGE_TTL);
} else {
    page_tail_main();
}

?>
