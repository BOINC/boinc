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

// This is a template for your web site's front page.
// You are encouraged to customize this file,
// and to create a graphical identity for your web site.
// by customizing the header/footer functions in html/project/project.inc
// and picking a Bootstrap theme

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/news.inc");
require_once("../inc/cache.inc");
require_once("../inc/uotd.inc");
require_once("../inc/sanitize_html.inc");
require_once("../inc/text_transform.inc");
require_once("../project/project.inc");
require_once("../inc/bootstrap.inc");

$config = get_config();
$master_url = parse_config($config, "<master_url>");
$no_computing = parse_config($config, "<no_computing>");
$no_web_account_creation = parse_bool($config, "no_web_account_creation");
    
$stopped = web_stopped();
$user = get_logged_in_user(false);

// The panel at the top of the page
//
function panel_contents() {
    global $no_computing;
    if ($no_computing) {
        echo "
            XXX is a research project that uses volunteers
            to do research in XXX.
        ";
    } else {
        echo "
            XXX is a research project that uses Internet-connected
            computers to do research in XXX.
            You can participate by downloading and running a free program
            on your computer.
        ";
    }
    echo "
        <p>
        XXX is based at
        [describe your institution, with link to web page]
        <ul>
        <li> [Link to page describing your research in detail]
        <li> [Link to page listing project personnel, and an email address]
        </ul>
        <ul>
    ";
}

function top() {
    global $stopped, $master_url, $user;
    if ($stopped) {
        echo "
            <p class=\"lead text-center\">".PROJECT." is temporarily shut down for maintenance.</p>
        ";
    }
    panel(null, 'panel_contents');
}

function left(){
    global $no_computing, $no_web_account_creation, $master_url;
    panel(
        'Join ',
        function() {
            global $no_computing, $no_web_account_creation, $master_url;
            echo "<ul>";
            if ($no_computing) {
                echo "
                    <li> <a href=\"create_account_form.php\">Create an account</a>
                ";
            } else {
                echo "
                    <li><a href=\"info.php\">".tra("Read our rules and policies")."</a>
                    <li> This project uses BOINC.
                        If you're already running BOINC, select Add Project.
                        If not, <a target=\"_new\" href=\"http://boinc.berkeley.edu/download.php\">download BOINC</a>.
                    <li> When prompted, enter <br><b>".$master_url."</b>
                ";
                if (!$no_web_account_creation) {
                    echo "
                        <li> If you're running a command-line version of BOINC,
                            <a href=\"create_account_form.php\">create an account</a> first.
                    ";
                }
                echo "
                    <li> If you have any problems,
                        <a target=\"_new\" href=\"http://boinc.berkeley.edu/wiki/BOINC_Help\">get help here</a>.
                ";
            }
            echo "</ul>\n";
        }
    );
}

function news() {
    include("motd.php");
    show_news(0, 5);
}

function right() {
    global $stopped;
    if (!$stopped) {
        $profile = get_current_uotd();
        if ($profile) {
            echo "
                <div class=\"media uotd\">
            ";
            show_uotd($profile);
            echo "</div>\n";
        }
    }
    panel('News', 'news');
}

page_head(null, null, null, "", file_get_contents("schedulers.txt"));

grid('top', 'left', 'right');

page_tail(false, "", false);

?>
