<?php

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
        <th>Team name</th>
        <th>Description</th>
        <th>Average credit</th>
        <th>Type</th>
        <th>Country</th>
        </tr>
    ";
    foreach ($list as $team) {
        $type = team_type_name($team->type);
        echo "<tr class=bordered>
            <td class=shaded valign=top><a href=team_display.php?teamid=$team->id>$team->name</a></td>
            <td class=shaded valign=top><span class=note>".sanitize_html($team->description)."</span></td>
            <td class=shaded valign=top align=right>".format_credit($team->expavg_credit)."</td>
            <td class=shaded valign=top>$type</td>
            <td class=shaded valign=top>$team->country</td>
            </tr>
        ";
    }
    echo "</table>";
}

function search($params) {
    $list = array();
    $tried = false;
    if (strlen($params->keywords)) {
        $name_lc = strtolower($params->keywords);
        $name_lc = escape_pattern($name_lc);

        $list2 = get_teams("name='$name_lc'", $params->active);
        merge_lists($list2, $list, 20);

        $list2 = get_teams("name like '".boinc_real_escape_string($name_lc)."%'", $params->active);
        merge_lists($list2, $list, 5);

        $list2 = get_teams("match(name) against ('$params->keywords')", $params->active);
        merge_lists($list2, $list, 5);
        $list2 = get_teams("match(name, description) against ('$params->keywords')", $params->active);
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
        echo "
            No teams were found matching your criteria.
            Try another search.
            <p>
            Or you can <a href=team_create_form.php>create a new team</a>.
            <p>
        ";
        team_search_form($params);
    } else {
        echo "
            The following teams match one or more of your search criteria.
            To join a team, click its name to go to the team page,
               then click <b>Join this team</b>.
            <p>
        ";
        sort_list($list);
        show_list($list);
        echo "<h2>Change your search</h2>";
        team_search_form($params);
    }
}

$user = get_logged_in_user(false);
if (isset($_GET['submit'])) {
    $params = null;
    $params->keywords = $_GET['keywords'];
    $params->country = $_GET['country'];
    $params->type = $_GET['type'];
    $params->active = get_str('active', true);
    page_head("Team search results");
    search($params);
} else {
    page_head("Find a team", 'document.form.keywords.focus()');
    echo "
        You can team up with other people with similar interests,
        or from the same country, company, or school.
        <p>
        Use this form to find teams that might be right for you.
        <p>
    ";
    team_search_form($params);
    if (isset($_COOKIE['init'])) {
        echo "
            <p>
            <a href=home.php>Click here</a>
            if you're not interested in joining a team right now.
        ";
    }
}
page_tail();

?>
