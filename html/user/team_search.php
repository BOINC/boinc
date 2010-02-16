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

include_once("../inc/boinc_db.inc");
include_once("../inc/util.inc");
include_once("../inc/team.inc");
include_once("../inc/team_types.inc");

// Merge list1 into list2.
// list entries are of the form id => team,
// where team includes a field "refcnt".
// 
function merge_lists($list1, &$list2, $weight) {
    foreach($list1 as $team) {
        $id = $team->id;
        if (array_key_exists($id, $list2)) {
            $list2[$id]->refcnt += $weight;
        } else {
            $list2[$id] = $team;
            $list2[$id]->refcnt = $weight;
        }
    }
}

function compare($t1, $t2) {
    if ($t1->refcnt > $t2->refcnt) return -1;
    if ($t1->refcnt < $t2->refcnt) return 1;
    if ($t1->rnd > $t2->rnd) return -1;
    if ($t1->rnd < $t2->rnd) return 1;
    return 0;
}

// Sort list by decreasing refcnt
//
function sort_list(&$list) {
    foreach ($list as $a=>$b) $b->rnd = rand();
    usort($list, 'compare');
}

function get_teams($clause, $active) {
    $c2 = '';
    if ($active) $c2 = "and expavg_credit>0.1";
    return BoincTeam::enum("$clause $c2 order by expavg_credit desc limit 20");
}

function show_list($list) {
    start_table();
    echo "
        <tr>
        <th>".tra("Team name")."</th>
        <th>".tra("Description")."</th>
        <th>".tra("Average credit")."</th>
        <th>".tra("Type")."</th>
        <th>".tra("Country")."</th>
        </tr>
    ";
    $i = 0;
    foreach ($list as $team) {
        $type = team_type_name($team->type);
        $j = $i++ % 2;
        echo "<tr class=row$j>
            <td><a href=team_display.php?teamid=$team->id>$team->name</a></td>
            <td><span class=note>".sanitize_html($team->description)."</span></td>
            <td>".format_credit($team->expavg_credit)."</td>
            <td>$type</td>
            <td>$team->country</td>
            </tr>
        ";
    }
    echo "</table>";
}

function search($params) {
    $list = array();
    $tried = false;
    if (strlen($params->keywords)) {
        $kw = BoincDb::escape_string($params->keywords);
        $name_lc = strtolower($kw);
        $name_lc = escape_pattern($name_lc);

        $list2 = get_teams("name='$name_lc'", $params->active);
        merge_lists($list2, $list, 20);

        $list2 = get_teams("name like '".$name_lc."%'", $params->active);
        merge_lists($list2, $list, 5);

        $list2 = get_teams("match(name) against ('$kw')", $params->active);
        merge_lists($list2, $list, 5);
        $list2 = get_teams("match(name, description) against ('$kw')", $params->active);
        //echo "<br>keyword matches: ",sizeof($list2);
        merge_lists($list2, $list, 3);
        $tried = true;
    }
    if (strlen($params->country) && $params->country!='None') {
        $list2 = get_teams("country = '$params->country'", $params->active);
        //echo "<br>country matches: ",sizeof($list2);
        merge_lists($list2, $list, 1);
        $tried = true;
    }
    if ($params->type and $params->type>1) {
        $list2 = get_teams("type=$params->type", $params->active);
        //echo "<br>type matches: ",sizeof($list2);
        merge_lists($list2, $list, 2);
        $tried = true;
    }

    if (!$tried) {
        $list = get_teams("id>0", $params->active);
    }

    if (sizeof($list) == 0) {
        echo tra("No teams were found matching your criteria. Try another search.")
            ."<p>"
            .tra("Or you can %1create a new team%2.", "<a href=team_create_form.php>", "</a>")
            ."</p>\n";
        team_search_form($params);
    } else {
        echo tra("The following teams match one or more of your search criteria.
            To join a team, click its name to go to the team page,
               then click %1Join this team%2.", "<strong>", "</strong>")
            ."<p>
        ";
        sort_list($list);
        show_list($list);
        echo "<h2>".tra("Change your search")."</h2>";
        team_search_form($params);
    }
}

$user = get_logged_in_user(false);
if (isset($_GET['submit'])) {
    $params = null;
    $params->keywords = get_str('keywords', true);
    $params->country = $_GET['country'];
    $params->type = $_GET['type'];
    $params->active = get_str('active', true);
    page_head(tra("Team search results"));
    search($params);
} else {
    page_head(tra("Find a team"), 'document.form.keywords.focus()');
    echo tra("You can team up with other people with similar interests, or from the same country, company, or school.")
        ."<p>"
        .tra("Use this form to find teams that might be right for you.")
        ."</p>\n";
    team_search_form($params);
    if (isset($_COOKIE['init'])) {
        echo "<p>
            ".tra("%1I'm not interested%2 in joining a team right now.", "<a href=home.php>", "</a>");
    }
}
page_tail();

?>
