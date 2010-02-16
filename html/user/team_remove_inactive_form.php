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
require_once("../inc/team.inc");

ini_set("memory_limit", "1024M");

$logged_in_user = get_logged_in_user();
$teamid = get_int("teamid");
$team = BoincTeam::lookup_id($teamid);
if (!$team) error_page("no such team");
require_admin($logged_in_user, $team);
page_head(tra("Remove members from %1", $team->name));
echo "
    <form method=\"post\" action=\"team_remove_inactive_action.php\">
    <input type=\"hidden\" name=\"id\" value=\"".$team->id."\">
";
start_table();
echo "<tr>
    <th>".tra("Remove?")."</th>
    <th>".tra("Name (ID)")."</th>
    <th>".tra("Total credit")."</th>
    <th>".tra("Recent average credit")."</th>
    </tr>
";

$users = BoincUser::enum("teamid=$team->id");
$ninactive_users = 0;
foreach($users as $user) {
    if ($user->id == $logged_in_user->id) continue;
    if ($user->id == $team->userid) continue;
    $user_total_credit = format_credit($user->total_credit);
    $user_expavg_credit = format_credit($user->expavg_credit);
    echo "
        <tr>
        <td align=center><input type=checkbox name=remove_$ninactive_users value=$user->id>
        <td>".user_links($user)." ($user->id)</td>
        <td>$user_total_credit</td>
        <td>$user_expavg_credit</td>
        </tr>
    ";
    $ninactive_users++;
}
end_table();
if ($ninactive_users == 0) {
    echo "<p>".tra("No members are eligible for removal.")."</p>";
} else {
    echo "<input type=hidden name=ninactive_users value=$ninactive_users>";
    echo "<input type=submit value=\"".tra("Remove users")."\">";
}
echo "</form>";
page_tail();
?>
