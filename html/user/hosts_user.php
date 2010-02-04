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

// show all the hosts for a user.
// if $userid is absent, show hosts of logged-in user

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/host.inc");
require_once("../inc/cache.inc");

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
    $user = lookup_user_id($userid);
    if (!$user) {
        error_page("No such user");
    }
    $caching = true;

    // At this point, we know that $userid, $show_all and $sort all have
    // valid values.
    //
    $cache_args="userid=$userid&amp;show_all=$show_all&amp;sort=$sort&amp;rev=$rev";
    start_cache(USER_PAGE_TTL, $cache_args);
    if ($user->show_hosts) {
        page_head("Computers belonging to $user->name");
    } else {
        page_head("Computers hidden");
        echo "This user has chosen not to show information about their computers.\n";
        page_tail();
        end_cache(USER_PAGE_TTL, $cache_args);
        exit();
    }
    $private = false;
} else {
    $user = get_logged_in_user();
    $caching = false;
    $userid = $user->id;
    page_head("Your computers");
    $private = true;
}

show_user_hosts($userid, $private, $show_all, $sort, $rev);

if ($caching) {
    page_tail(true);
    end_cache(USER_PAGE_TTL, $cache_args);
} else {
    page_tail();
}

?>
