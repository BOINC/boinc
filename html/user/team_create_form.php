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

$user = get_logged_in_user();

page_head(tra("Create a team"));
if ($user->teamid) {
    $team = BoincTeam::lookup_id($user->teamid);
    echo tra("You belong to %1. You must %2quit this team%3 before creating a new one.", "<a href=\"team_display.php?teamid=".$team->id."\">".$team->name."</a>", "<a href=\"team_quit_form.php\">", "</a>");
} else {
    team_edit_form(null, tra("Create a team"), "team_create_action.php");
}
page_tail();
?>
