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

check_get_args(array());

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
        echo "<p> "
        .tra("If you have not already done so, %1 download BOINC client software %2.", "<a href=\"https://boinc.berkeley.edu/download.php\">", "</a>")."</p>";
    }
} else {
    page_head(tra("Your account"));
}

show_account_private($user);

page_tail();

?>
