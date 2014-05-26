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

check_get_args(array());

$user = get_logged_in_user();
$teamid = post_int("id");
$team = BoincTeam::lookup_id($teamid);
if (!$team) error_page(tra("No such team"));
require_founder_login($user, $team);

page_head(tra("Removing users from %1", $team->name));
$ndel = 0;
for ($i=0; $i<$_POST["ninactive_users"]; $i++) {
    $userid = post_int("remove_$i", true);
    if (!$userid) continue;
    $user = BoincUser::lookup_id($userid);
    if (!$user) continue;
    if ($user->teamid != $team->id) {
        echo "<br />".tra("%1 is not a member of %2", $user->name, $team->name);
    } else {
        user_quit_team($user);
        echo "<br />".tra("%1 has been removed", $user->name);
        $ndel++;
    }
}

page_tail();

?>
