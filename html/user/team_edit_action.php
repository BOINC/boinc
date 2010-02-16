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
require_once("../inc/team.inc");
require_once("../inc/sanitize_html.inc");
require_once("../inc/boinc_db.inc");

$user = get_logged_in_user();
$teamid = post_int("teamid");
$team = BoincTeam::lookup_id($teamid);

if (!$team) error_page(tra("no such team"));
require_admin($user, $team);

$team_url = BoincDb::escape_string(strip_tags(post_str("url", true)));
$x = strstr($team_url, "http://");
if ($x) {
    $team_url = substr($team_url, 7);
}
$team_name = BoincDb::escape_string(strip_tags(post_str("name")));
$team_name_lc = strtolower($team_name);

$tnh = post_str("name_html", true);
$team_name_html = sanitize_html($tnh);

$team_name_html = BoincDb::escape_string($team_name_html);

$team_description = BoincDb::escape_string(post_str("description", true));
$type = BoincDb::escape_string(post_str("type", true));
$country = BoincDb::escape_string(post_str("country", true));
if ($country == "") {
    $country = "International";
}
if (!is_valid_country($country)) {
    error_page(tra("bad country"));
}
$joinable = post_str('joinable', true)?1:0;

$t = BoincTeam::lookup("name='$team_name'");
if ($t && $t->id != $teamid) {
    error_page(tra("The name '%1' is being used by another team.", $team_name));
}
if (strlen($team_name) == 0) {
    error_page(tra("Must specify team name"));
    // Should be caught up with the post_str("name"),
    // but you can never be too safe.
}

$clause = sprintf(
    "name = '%s',
    name_lc = '%s',
    name_html = '%s',
    url = '%s',
    description = '%s',
    type = %d,
    country='%s',
    joinable=%d",
    $team_name,
    $team_name_lc,
    $team_name_html,
    $team_url,
    $team_description,
    $type,
    $country,
    $joinable
);

$ret = $team->update($clause);
if ($ret) {
    Header("Location: team_display.php?teamid=$team->id");
} else {
    error_page(tra("Could not update team - please try again later."));
}

?>
