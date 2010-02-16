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
require_once("../inc/boinc_db.inc");

$user = get_logged_in_user();

$teamid = post_int("teamid");
$team = BoincTeam::lookup_id($teamid);
if (!$team) {
    error_page(tra("no such team"));
}
require_founder_login($user, $team);
check_tokens($user->authenticator);

$userid = post_int("userid");
$new_founder = BoincUser::lookup_id($userid);
if (!$new_founder || $new_founder->teamid != $team->id) {
    error_page(tra("User is not a member of %1", $team->name));
}

page_head(tra("Changing founder of %1", $team->name));
$team->update("userid=$userid, ping_user=0");
echo tra("%1 is now founder of %2", $new_founder->name, $team->name);
page_tail();

?>
