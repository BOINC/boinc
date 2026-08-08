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

// Show member list.
// Name is outdated; don't show emails any more

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../inc/team.inc");

$xml = get_int('xml', true);
if ($xml) {
    require_once("../inc/xml.inc");
}

if (DISABLE_TEAMS) {
    if ($xml) {
        xml_error(-1, "Teams are disabled");
    } else {
        error_page("Teams are disabled");
    }
}

BoincDb::get(true);
if ($xml) {
    $creditonly = get_int('creditonly', true);
    xml_header();
    $retval = db_init_xml();
    if ($retval) xml_error($retval);
    $teamid = get_int("teamid");
    $team = BoincTeam::lookup_id($teamid);
    if (!$team) {
        xml_error(ERR_DB_NOT_FOUND);
    }
    echo "<users>\n";
    $users = BoincUser::enum_fields("id, email_addr, send_email, name, total_credit, expavg_credit, expavg_time, has_profile, donated, country, cross_project_id, create_time, url", "teamid=$team->id");
    foreach($users as $user) {
        show_team_member($user, $creditonly);
    }
    echo "</users>\n";
    exit();
}

$user = get_logged_in_user();
$teamid = get_int("teamid");
$plain = get_int("plain", true);
$team = BoincTeam::lookup_id($teamid);
if (!$team) error_page(tra("no such team"));
require_founder_login($user, $team);

if ($plain) {
    header("Content-type: text/plain");
} else {
    page_head(tra("Members of %1", $team->name));
    start_table('table-striped');
    row_heading_array(
        array(
            tra("Name"),
            tra("ID"),
            tra("Total credit"),
            tra("Recent average credit"),
            tra("Country")
        )
    );
}

$users = BoincUser::enum_fields("id, email_addr, send_email, name, total_credit, expavg_credit, has_profile, donated, country, cross_project_id, create_time, url", "teamid=$team->id");
foreach($users as $user) {
    if ($plain) {
        echo "$user->name $user->id\n";
    } else {
        table_row(user_links($user, BADGE_HEIGHT_MEDIUM), $user->id, format_credit($user->total_credit), format_credit($user->expavg_credit), $user->country);
    }
}
if (!$plain) {
    end_table();
    echo "<p><a href=\"team_email_list.php?teamid=".$teamid."&amp;plain=1\">".tra("Show as plain text")."</a></p>";
    page_tail();
}

?>
