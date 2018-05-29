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
    
$stopped = web_stopped();
$user = get_logged_in_user(false);

// The panel at the top of the page
//
function panel_contents() {
}

function top() {
    global $stopped, $master_url, $user;
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
    global $user, $no_web_account_creation, $master_url;
    panel(
        $user?tra("Welcome, %1", $user->name):tra("What is %1?", PROJECT),
        function() use($user) {
            global $no_web_account_creation, $master_url;
            if ($user) {
                $dt = time() - $user->create_time;
                if ($dt < 86400) {
                    echo tra("Thanks for joining %1", PROJECT);
                } else if ($user->total_credit == 0) {
                    echo "Your computer hasn't completed any tasks yet.  If you need help, <a href=https://boinc.berkeley.edu/help.php>go here</a>";
                } else {
                    $x = format_credit($user->expavg_credit);
                    $y = number_format($user->expavg_credit/200, 3);
                    echo tra("You've contributed about %1 credits per day (%2 GFLOPS) to %3 recently.", $x, $y, PROJECT);
                    if ($user->expavg_credit > 1) {
                        echo "Thanks!";
                    } else {
                        echo "<p><p>";
                        echo "Please make sure BOINC is installed and enabled on your computer.";
                    }
                    echo "<p>";
                }
                echo sprintf('<center><a href=home.php class="btn btn-success">%s</a></center>
                    ',
                    tra('Continue to your home page')
                );
            } else {
                echo "
                    <p>
                    XXX is a research project, based at <a href=#>YYY</a>,
                    that uses Internet-connected
                    computers to do research in XXX.
                    You can contribute to our research
                    by running a free program on your computer.
                    </p>
                ";
                echo "
                    <ul>
                    <li> <a href=#>Our research</a>
                    <li> <a href=#>Our team</a>
                    </ul>
                ";
                if (NO_COMPUTING) {
                    echo "
                        <a href=\"create_account_form.php\">Create an account</a>
                    ";
                } else {
                    echo '<center><a href="signup.php" class="btn btn-success"><font size=+2>'.tra('Join %1', PROJECT).'</font></a></center>
                    ';

                }
            }
        }
    );
    global $stopped;
    if (!$stopped) {
        $profile = get_current_uotd();
        if ($profile) {
            panel('User of the Day',
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
