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

require_once("../inc/util.inc");
require_once("../inc/user.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/forum.inc");

// show the home page of logged-in user

$user = get_logged_in_user();
BoincForumPrefs::lookup($user);
$user = get_other_projects($user);

$init = isset($_COOKIE['init']);
$via_web = isset($_COOKIE['via_web']);
if ($via_web) {
    clear_cookie('via_web');
}

$cache_control_extra = "no-store,";

if ($init) {
    clear_cookie('init');
    page_head(tra("Welcome to %1", PROJECT));
    echo "<p>".tra("View and edit your account preferences using the links below.")."</p>\n";
    if ($via_web) {
        echo "
            <p> If you have not already done so,
            <a href=\"http://boinc.berkeley.edu/download.php\">download BOINC client software</a>.</p>
        ";
    }
} else {
    page_head(tra("Your account"));
}

start_table_noborder();
echo "<tr><td valign=top>";
start_table();
show_user_info_private($user);
if (!no_computing()) {
    show_user_stats_private($user);
}

if (file_exists("../project/donations.inc")) {
    require_once("../project/donations.inc");
    if (function_exists('show_user_donations_private')) {
        show_user_donations_private($user);
    }
}
end_table();
show_other_projects($user, true);
project_user_page_private($user);
echo "</td><td valign=top>";
start_table();
show_community_private($user);
end_table();

echo "</td></tr></table>";

page_tail();

?>
