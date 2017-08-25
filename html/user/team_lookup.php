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

check_get_args(array("format", "team_id", "team_ids", "team_name"));

$format = get_str("format", true);
$team_id = get_int("team_id", true);
$team_ids = get_str("team_ids", true);

BoincDb::get(true);

if ($team_id || $team_ids || ($format == 'xml')) {
    require_once ('../inc/xml.inc');
    xml_header();
    $retval = db_init_xml();
    if ($retval) xml_error($retval);
}

if ($team_id) {
    $team = BoincTeam::lookup_id($team_id);
    if ($team) {
        show_team_xml($team);
    } else {
        xml_error(ERR_DB_NOT_FOUND);
    }
    exit();
}

if ($team_ids) {
    $team_id_array = explode(",", $team_ids);
    echo "<teams>\n";
    $total = 0;
    foreach ($team_id_array as $team_id) {
        if (is_numeric($team_id)) { //make sure only numbers get through
            $team = BoincTeam::lookup_id($team_id);
            if ($team) {
                show_team_xml($team);
                $total++;
                if ($total == 100) break;
            }
            //do not error out
        }
    }
    echo "</teams>\n";
    exit();
}

$team_name = get_str("team_name");
$name_lc = strtolower($team_name);
$name_lc = escape_pattern($name_lc);

$clause = "name like '%".BoincDb::escape_string($name_lc)."%' order by expavg_credit desc limit 100";
$teams = BoincTeam::enum($clause);

if ($format == 'xml') {
    echo "<teams>\n";
    $total = 0;
    foreach($teams as $team) {
        show_team_xml($team);
        $total++;
        if ($total == 100) break;
    }
    echo "</teams>\n";
    exit();
}

page_head(tra("Search Results"));
if (count($teams)) {
    echo "<h2>".tra("Search results for '%1'", sanitize_tags($team_name))."</h2>";
    echo "<p>";
    echo tra("You may view these teams' members, statistics, and information.");
    echo "<ul>";
    foreach($teams as $team) {
        echo "<li>";
        echo "<a href=team_display.php?teamid=$team->id>";
        echo "$team->name</a></li>";
    }
    echo "</ul>";
    if (count($teams)==100) {
        echo
            tra("More than 100 teams match your search. The first 100 are shown.")
            ."<br>
        ";
    }
}
echo tra(
    "End of results. %1 If you cannot find the team you are looking for, you may %2 create a team %3 yourself.",
    "<br>",
    "<a href=team_create_form.php>",
    "</a>"
);
page_tail();

?>
