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

$user = get_logged_in_user(true);
check_tokens($user->authenticator);

$teamid = get_int("teamid");
$team = BoincTeam::lookup_id($teamid);
require_team($team);
if (!$team->joinable) {
    error_page("The team is not joinable.");
}
if ($user->teamid == $team->id) {
    page_head("Already a member");
    echo "You are already a member of $team->name.";
} else {
    $success = user_join_team($team, $user);
    if ($success) {
        Header("Location: home.php");
    } else {
        error_page("Couldn't join team - please try later.");
    }
}

page_tail();

?>
