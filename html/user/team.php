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

echo "<p>".tra("%1 participants may form %2teams%3", PROJECT, "<b>", "</b>") ."
    <p>
    ". tra("You may belong to only one team. You can join or quit a team at any time."). "
    <p>" .
    tra("Each team has a %1founder%2 who may:", "<b>", "</b>") . "
    <ul>
    <li> " . tra("access team members' email addresses") . "
    <li> " . tra("edit the team's name and description") . "
    <li> " . tra("remove members from the team") . "
    <li> " . tra("disband a team if it has no members") . "
    </ul>
    <p>" .
    tra("To join a team, visit its team page and click %1Join this team%2.", "<b>", "</b>") . "
    <h3>" . tra("Find a team") . "</h3>
";
team_search_form(null);
echo "

    <h3>" . tra("Top teams") . "</h3>
    <ul>
    <li> <a href=\"top_teams.php\">" . tra("All teams") . "</a>
";

for ($i=1; $i<8; $i++) {
    echo "<li> <a href=\"top_teams.php?type=".$i."\">".tra("%1 teams", team_type_name($i))."</a>
    ";
}

echo "
    </ul>
    <h3>" . tra("Create a new team") . "</h3>
    " . tra("If you cannot find a team that is right for you, you can %1create a team%2.", "<a href=\"team_create_form.php\">","</a>");
page_tail();

?>
