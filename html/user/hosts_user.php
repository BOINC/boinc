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

// show all the hosts for a user.
// if $userid is absent, show hosts of logged-in user

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/host.inc");

check_get_args(array("show_all", "rev", "sort", "userid"));

$show_all = get_int("show_all", true);
if ($show_all != 1) {
    $show_all = 0;
}

$rev = get_int("rev", true);
if ($rev != 1) {
    $rev = 0;
}

$sort = get_str("sort", true);

$user = get_logged_in_user(false);
$userid = get_int("userid", true);

if ($user && $user->id == $userid) {
    $userid = 0;
}
if ($userid) {
    $user = BoincUser::lookup_id($userid);
    if (!$user) {
        error_page("No such user");
    }

    if ($user->show_hosts) {
        page_head(tra("Computers belonging to %1", $user->name));
    } else {
        page_head(tra("Computers hidden"));
        echo tra(tra("This user has chosen not to show information about his or her computers."));
        page_tail();
        exit();
    }
    $private = false;
} else {
    $user = get_logged_in_user();
    $userid = $user->id;
    page_head(tra("Your computers"));
    $private = true;
}

show_user_hosts($userid, $private, $show_all, $sort, $rev);

page_tail();

?>
