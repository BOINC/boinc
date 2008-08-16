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

include_once("../inc/db.inc");
include_once("../inc/util.inc");
include_once("../inc/team.inc");

page_head("Teams");

echo "<p>".PROJECT." participants may form <b>teams</b>.
    <p>
    You may belong to only one team.
    You can join or quit a team at any time.
    <p>
    Each team has a <b>founder</b>, who may
    <ul>
    <li> access team members' email addresses
    <li> edit the team's name and description
    <li> remove members from the team
    <li> disband a team if it has no members
    </ul>
    <p>
    To join a team, visit its team page and click <b>Join this team</b>.
    <h3>Find a team</h3>
";
team_search_form(null);
echo "

    <h3>Top teams</h3>
    <ul>
    <li> <a href=top_teams.php>All teams</a>
";

for ($i=1; $i<8; $i++) {
    echo "<li> <a href=\"top_teams.php?type=$i\">".team_type_name($i)." teams</a>
    ";
}

echo "
    </ul>
    <h3>Create a new team</h3>
    If you can't find a team that's right for you, you can
    <a href=team_create_form.php>create a team</a>.
";
page_tail();

?>
