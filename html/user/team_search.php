<?php

include_once("../inc/db.inc");
include_once("../inc/util.inc");
include_once("../inc/team.inc");
include_once("../inc/team_types.inc");

db_init();

function print_form($user) {
    echo "
        <form name=form action=team_search.php>
    ";
    start_table();
    row2("Key words<br><span class=note>Find teams with these words in their names or descriptions</span>", "<input name=keywords>");
    row2_init("Country", "");
    echo "<select name=country><option value=\"\" selected>---</option>";
    $country = $user->country;
    if (!$country || $country == 'None') $country = "XXX";
    print_country_select($country);
    echo "</select></td></tr>\n";
    row2("Type of team", team_type_select(0, true));
    row2("Show only active teams", "<input type=checkbox name=active checked>");
    row2("", "<input type=submit name=submit value=Search>");
    end_table();
    echo "
        </form>
    ";
}

// Merge list1 into list2.
// list entries are of the form id => team,
// where team includes a field "refcnt".
// 
function merge_lists($list1, &$list2) {
    foreach($list1 as $id=>$team) {
        if (array_key_exists($id, $list2)) {
            $list2[$id]->refcnt++;
        } else {
            $list2[$id] = $team;
            $list2[$id]->refcnt = 0;
        }
    }
}

function compare($t1, $t2) {
    if ($t1->refcnt > $t2->refcnt) return -1;
    if ($t1->refcnt < $t2->refcnt) return 1;
    if ($t1->expavg_credit > $t2->rnd) return -1;
    if ($t1->expavg_credit < $t2->rnd) return 1;
    return 0;
}

// Sort list by decreasing refcnt
//
function sort_list(&$list) {
    foreach ($list as $a=>$b) $b->rnd = rand();
    usort($list, compare);
}

function get_teams($clause, $active) {
    if ($active) $c2 = "and expavg_credit>0.1";
    $query = "select * from team where $clause $c2 order by expavg_credit desc limit 20";
    $result = mysql_query($query);
    $list = array();
    while ($team = mysql_fetch_object($result)) {
        $list[$team->id] = $team;
    }
    return $list;
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

function search($user) {
    $keywords = $_GET['keywords'];
    $country = $_GET['country'];
    $type = $_GET['type'];
    $active = $_GET['active'];

    $list = array();
    if (strlen($keywords)) {
        $list2 = get_teams("match(name, description) against ('$keywords')", $active);
        //echo "<br>keyword matches: ",sizeof($list2);
        merge_lists($list2, $list);
    }
    if (strlen($country) && $country!='None') {
        $list2 = get_teams("country = '$country'", $active);
        //echo "<br>country matches: ",sizeof($list2);
        merge_lists($list2, $list);
    }
    if ($type and $type>1) {
        $list2 = get_teams("type=$type", $active);
        //echo "<br>type matches: ",sizeof($list2);
        merge_lists($list2, $list);
    }

    if (sizeof($list) == 0) {
        echo "
            No teams were found matching your criteria.
            Try another search.
            <p>
            Or you can <a href=team_create_form.php>create a new team</a>.
            <p>
        ";
        print_form($user);
    } else {
        echo "
            The following teams match your search criteria.
            To join a team, click its name to go to the team page,
               then click <b>Join this team</b>.
            <p>
        ";
        sort_list($list);
        show_list($list);
    }
}

$user = get_logged_in_user(false);
if ($_GET['submit']) {
    page_head("Team search results");
    search($user);
} else {
    page_head("Find a team", 'document.form.keywords.focus()');
    echo "
        You can team up with other people with similar interests,
        or from the same country, company, or school.
        <p>
        Use this form to find teams that might be right for you.
        <p>
    ";
    print_form($user);
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
