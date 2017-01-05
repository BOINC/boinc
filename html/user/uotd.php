<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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
require_once("../inc/uotd.inc");
require_once("../inc/profile.inc");

if (DISABLE_PROFILES) error_page("Profiles are disabled");

check_get_args(array());

db_init();

$profile = get_current_uotd();
if (!$profile) {
    page_head("No UOTD");
    echo tra("No user of the day has been chosen.");
} else {
    $d = gmdate("d F Y", time());
    $user = BoincUser::lookup_id($profile->userid);
    page_head(tra("User of the Day for %1: %2", $d, $user->name));
    start_table();
    show_profile($user, get_logged_in_user(false));
    end_table();
}

page_tail();
?>
