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

function show_profile_link($profile, $n) {
    $user = lookup_user_id($profile->userid);
    echo "<tr><td align=\"center\">".user_links($user)."</td><td align=\"center\">".date_str($user->create_time)."</td><td align=\"center\">".$user->country."</td><td align=\"center\">".(int)$user->total_credit."</td><td align=\"center\">".(int)$user->expavg_credit."</td></tr>\n";
}

$search_string = get_str('search_string');
$offset = get_int('offset', true);
if (!$offset) $offset=0;
$count = 10;

page_head("Profile search results");

echo "<h2>Profiles containing '$search_string'</h2>\n";
$profiles = BoincProfile::enum("match(response1, response2) against ('$search_string') limit $offset,$count");
echo "<table align=\"center\" cellpadding=\"1\" border=\"1\" width=\"90%\">
    <tr><th align=\"center\">User name</th>
    <th align=\"center\">Joined project</th>
    <th align=\"center\">Country</th>
    <th  align=\"center\">Total credit</th>
    <th  align=\"center\">Recent credit</th></tr>
";
$n = 0;
foreach($profiles as $profile) {
    show_profile_link($profile, $n+$offset+1);
    $n += 1;
}
echo "</table>";

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
