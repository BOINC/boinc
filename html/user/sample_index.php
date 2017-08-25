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
        tra("What is %1?", PROJECT),
        function() use($user) {
            global $no_web_account_creation, $master_url;
            if (NO_COMPUTING) {
                echo "
                    XXX is a research project that uses volunteers
                    to do research in XXX.
                ";
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
            }
            echo "
                <ul>
                <li> <a href=#>Our research</a>
                <li> <a href=#>Our team</a>
                </ul>
            ";
            echo "</ul>";
            if (!$user) {
                if (NO_COMPUTING) {
                    echo "
                        <a href=\"create_account_form.php\">Create an account</a>
                    ";
                } else {
                    echo '<center><a href="join.php" class="btn btn-success"><font size=+2>'.tra('Join %1', PROJECT).'</font></a></center>
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
