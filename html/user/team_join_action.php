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
require_once("../inc/util.inc");
require_once("../inc/team.inc");

if (DISABLE_TEAMS) error_page("Teams are disabled");

check_get_args(array("tnow", "ttok"));

$user = get_logged_in_user(true);
check_tokens($user->authenticator);

$teamid = post_int("teamid");
$team = BoincTeam::lookup_id($teamid);
require_team($team);
if (!$team->joinable) {
    error_page(tra("The team %1 is not joinable.", $team->name));
}
if ($user->teamid == $team->id) {
    page_head(tra("Already a member"));
    echo tra("You are already a member of %1.", $team->name);
} else {
    $success = user_join_team($team, $user);
    if ($success) {
        page_head(tra("Joined %1", $team->name));
        echo tra("You have joined %1.", "<a href=team_display.php?teamid=$team->id>$team->name</a>");
    } else {
        error_page(tra("Couldn't join team - please try again later."));
    }
}

page_tail();

?>
