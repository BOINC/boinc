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

require_once("../inc/util.inc");
require_once("../inc/team.inc");

if (DISABLE_TEAMS) error_page("Teams are disabled");

check_get_args(array("id"));

$user = get_logged_in_user();
$teamid = get_int("id");

$team = BoincTeam::lookup_id($teamid);
if (!$team->joinable) {
    error_page(tra("The team %1 is not joinable.", $team->name));
}
$team_name = $team->name;
page_head(tra("Join %1", $team_name));
echo " <p><b>".tra("Please note:")."</b>
    <ul>
    <li>".tra("Joining a team gives its founder access to your email address.")."
    <li>".tra("Joining a team does not affect your account's credit.")."
    </ul>
    <hr>
    <form method=\"post\" action=\"team_join_action.php\">";
echo form_tokens($user->authenticator);
echo "
    <input type=\"hidden\" name=\"teamid\" value=\"$teamid\">
    <input class=\"btn btn-success\" type=\"submit\" value=\"".tra("Join team")."\">
    </form>
";
page_tail();

?>
