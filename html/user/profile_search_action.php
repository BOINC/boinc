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

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");

function show_profile_link2($profile, $n) {
    $user = lookup_user_id($profile->userid);
    echo "<tr><td>".user_links($user)."</td><td>".date_str($user->create_time)."</td><td>$user->country</td><td>".(int)$user->total_credit."</td><td>".(int)$user->expavg_credit."</td></tr>\n";
}

$search_string = get_str('search_string');
$search_string = strip_tags($search_string);
$search_string = boinc_real_escape_string($search_string);
$offset = get_int('offset', true);
if (!$offset) $offset=0;
$count = 10;

page_head("Profiles containing '$search_string'");
$profiles = BoincProfile::enum("match(response1, response2) against ('$search_string') limit $offset,$count");
start_table();
echo "
    <tr><th>User name</th>
    <th>Joined project</th>
    <th>Country</th>
    <th>Total credit</th>
    <th>Recent credit</th></tr>
";
$n = 0;
foreach($profiles as $profile) {
    show_profile_link2($profile, $n+$offset+1);
    $n += 1;
}
end_table();

if ($offset==0 && $n==0) {
    echo "No profiles found containing '$search_string'";
}

if ($n==$count) {
    $s = urlencode($search_string);
    $offset += $count;
    echo "
        <a href=profile_search_action.php?search_string=$s&offset=$offset>Next $count</a>
    ";

}

page_tail();
?>
