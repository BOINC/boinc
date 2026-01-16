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
//
// If you add text, put it in tra() to make it translatable.

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
$no_web_account_creation = parse_bool($config, "no_web_account_creation");
$project_id = parse_config($config, "<project_id>");

$stopped = web_stopped();
$user = get_logged_in_user(false);

// The panel at the top of the page
//
function panel_contents() {
}

function top() {
    global $stopped, $user;
    if ($stopped) {
        echo '
            <p class="lead text-center">'
            .tra("%1 is temporarily shut down for maintenance.", PROJECT)
            .'</p>
        ';
    }
    //panel(null, 'panel_contents');
}

function left(){
    global $user, $no_web_account_creation, $project_id;
    panel(
        $user?tra("Welcome, %1", $user->name):tra("What is %1?", PROJECT),
        function() use($user) {
            global $no_web_account_creation, $project_id;
            if ($user) {
                $dt = time() - $user->create_time;
                if ($dt < 86400) {
                    echo tra("Thanks for joining %1", PROJECT);
                } else if ($user->total_credit == 0) {
                    echo tra("Your computer hasn't completed any tasks yet.  If you need help, %1go here%2.",
                            "<a href=https://boinc.berkeley.edu/help.php>",
                            "</a>"
                    );
                } else {
                    $x = format_credit($user->expavg_credit);
                    echo tra("You've contributed about %1 credits per day to %2 recently.", $x, PROJECT);
                    if ($user->expavg_credit > 1) {
                        echo " ";
                        echo tra("Thanks!");
                    } else {
                        echo "<p><p>";
                        echo tra("Please make sure BOINC is installed and enabled on your computer.");
                    }
                }
                echo "<p><p>";
                echo sprintf('<center>%s</center>',
                    button_text('home.php',
                        tra('Continue to your home page'),
                        '',
                        '',
                        button_style('green', 16)
                    )
                );
                echo "<p><p>";
                echo sprintf('%s
                    <ul>
                    <li> %s
                    <li> %s
                    <li> %s
                    ',
                    tra("Want to help more?"),
                    tra("If BOINC is not installed on this computer, %1download it%2.",
                        "<a href=download_software.php>", "</a>"
                    ),
                    tra("Install BOINC on your other computers, tablets, and phones."),
                    tra("Tell your friends about BOINC, and show them how to join %1.", PROJECT)
                );
                if (function_exists('project_help_more')) {
                    project_help_more();
                }
                echo "</ul>\n";
            } else {
                echo "<p>";
                $pd = "../project/project_description.php";
                if (file_exists($pd)) {
                    include($pd);
                } else {
                    echo "No project description yet. Create a file html/project/project_description.php
                        that prints a short description of your project.
                    ";
                }
                echo "</p>\n";
                if (NO_COMPUTING) {
                    if (!$no_web_account_creation) {
                        echo "
                            <a href=\"create_account_form.php\">Create an account</a>
                        ";
                    }
                } else {
                    // use auto-attach if possible
                    //
                    if (!$no_web_account_creation) {
                        echo '<center><a href="signup.php" class="btn btn-success"><font size=+2>'.tra('Join %1', PROJECT).'</font></a></center>';
                    }
                    echo "<p><p>".tra("Already joined? %1Log in%2.",
                        "<a href=login_form.php>", "</a>"
                    );
                }
            }
        }
    );
    global $stopped;
    if (!$stopped) {
        $profile = get_current_uotd();
        if ($profile) {
            panel(tra('User of the Day'),
                function() use ($profile) {
                    show_uotd($profile);
                }
            );
        }
    }
}

function right() {
    panel(tra('News'),
        function() {
            include("motd.php");
            if (!web_stopped()) {
                show_news(0, 5);
            }
        }
    );
}

page_head(null, null, true);

grid('top', 'left', 'right');

page_tail(false, "", true);

?>
