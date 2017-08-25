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

require_once("../inc/boinc_db.inc");
require_once("../inc/user.inc");
require_once("../inc/team.inc");

if (DISABLE_TEAMS) error_page("Teams are disabled");

check_get_args(array("xml", "teamid"));

$xml = get_int('xml', true);

function show_delta($delta) {
    global $xml;
    $user = BoincUser::lookup_id($delta->userid);
    $when = time_str($delta->timestamp);
    $what = $delta->joining?"joined":"quit";
    if ($xml) {
        echo "    <action>
        <id>$user->id</id>
        <name>$user->name</name>
        <action>$what</action>
        <total_credit>$delta->total_credit</total_credit>
        <when>$when</when>
    </action>
";
    } else {
        echo "<tr>
           <td>$when</td>
           <td>",user_links($user, BADGE_HEIGHT_MEDIUM)," (ID $user->id)</td>
           <td>$what</td>
           <td>$delta->total_credit</td>
           </tr>
        ";
    }
}

$user = get_logged_in_user();
$teamid = get_int('teamid');
$team = BoincTeam::lookup_id($teamid);
if ($xml) {
    require_once('../inc/xml.inc');
    xml_header();
}

if (!$team || !is_team_admin($user, $team)) {
    if ($xml) {
        xml_error("-1", "Not founder or admin");
    } else {
        error_page(tra("Not founder or admin"));
    }
}

if ($xml) {
    echo "<actions>\n";
} else {
    page_head(tra("Team history for %1", $team->name));
    start_table();
    row_heading_array(
        array(
            tra("When"),
            tra("User"),
            tra("Action"),
            tra("Total credit at time of action"),
        )
    );
}
$deltas = BoincTeamDelta::enum("teamid=$teamid order by timestamp");
foreach($deltas as $delta) {
    show_delta($delta);
}
if ($xml) {
    echo "</actions>\n";
} else {
    end_table();
    page_tail();
}

?>
