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
 
require_once("../inc/profile.inc");

$userid = get_int('userid');
$user = BoincUser::lookup_id($userid);
if (!$user) {
    error_page("No such user");
}
if (!$user->has_profile) {
    error_page("No profile");
}
 
$logged_in_user = get_logged_in_user(false);
$caching = false;
if (!$logged_in_user) {
    $caching = true;
    $cache_args = "userid=$userid";
    start_cache(USER_PROFILE_TTL, $cache_args);
}
page_head("Profile: $user->name");
start_table();
echo "<tr><td valign=top>";
start_table();
show_profile($user, $logged_in_user);
end_table();
echo "</td><td valign=top>";
start_table();
row2("Account data", "<a href=show_user.php?userid=$userid>View</a>");
community_links($user);
end_table();
echo "</td></tr></table>";

page_tail();

if ($caching) {
    end_cache(USER_PROFILE_TTL, $cache_args);
}

?>
