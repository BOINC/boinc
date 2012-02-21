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

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

check_get_args(array());

db_init();
$user = get_logged_in_user(true);

$team = lookup_team($user->teamid);
if (!$team) {
    error_page(tra("No such team"));
}

page_head(tra("Quit %1", $team->name));
echo tra("<strong>Please note before quitting a team:</strong>
         <ul>
         <li>If you quit a team, you may rejoin later, or join any other team you desire
         <li>Quitting a team does not affect your personal credit statistics in any way.
         </ul>")
    ."<form method=\"post\" action=\"team_quit_action.php\">";
echo form_tokens($user->authenticator);
echo "<input type=\"hidden\" name=\"id\" value=\"$team->id\">
    <input type=\"submit\" value=\"".tra("Quit Team")."\">
    </form>
";
page_tail();

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
