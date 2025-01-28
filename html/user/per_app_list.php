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

// show top users or teams, ordered by per-app credit
//
// URL args:
// is_team: if nonzero, show teams
// appid: ID of app for sorting; default is first app returned by enum
// is_total: if nonzero, sort by total credit

require_once("../inc/util.inc");
require_once("../inc/team.inc");

check_get_args(array(
    "is_team",
    "appid",
    "is_total",
    "offset"
));

define('ITEM_LIMIT', 10000);

// return a column title (Average or Total),
// hyperlinked if this is not the current sort column
//
function col_title($is_team, $app, $appid, $is_total, $i) {
    $x = $i ? tra("Total") : tra("Average");
    if ($app->id == $appid && ($is_total ? $i : !$i)) {
        return $x;
    } else {
        return "<a href=per_app_list.php?appid=$app->id&is_team=$is_team&is_total=$i>$x</a>";
    }
}

// print a row of app names,
// under each of which are columns for Average and Total
//
function show_header($is_team, $apps, $appid, $is_total) {
    echo "<tr><th colspan=2>&nbsp;</th>";
    foreach ($apps as $app) {
        echo "<th colspan=2 class=\"text-center\">$app->name</th>\n";
    }
    echo "</tr>";

    echo "<tr>";
    echo "<th>" . tra("Rank") . "</th><th>" . tra("Name") . "</th>\n";
    foreach ($apps as $app) {
        for ($i = 0; $i < 2; $i++) {
            $x = col_title($is_team, $app, $appid, $is_total, $i);
            echo "<th class=\"text-right\">$x</th>\n";
        }
    }

    echo "</tr>";

}

// show a user or team, with their credit for each app
//
function show_row($item, $x) {
    global $i;
    global $apps;
    echo "<tr>";
    echo "<td>$i</td>";

    echo "<td>" . $item[0][$x] . "</td>";
    $y = 1;
    foreach ($apps as $app) {
        $z = 0;
        echo "<td>" . format_credit($item[$y][$x][$z]) . "</td>";
        $z++;
        echo "<td>" . format_credit_large($item[$y][$x][$z]) . "</td>";
        $y++;
    }
    echo "</tr>";

}

function retrieve_credit_team($data) {
    global $apps;
    $x = 1;
    $c = 0;

    foreach ($data as $item) {
        $team = BoincTeam::lookup_id($item->teamid);
        if (is_object($team)) {
            $sign         = team_links($team);
            $store[0][$c] = $sign;
            $c++;
        }
    }

    foreach ($apps as $app) {
        $y = 0;

        foreach ($data as $item) {
            $team = BoincTeam::lookup_id($item->teamid);
            if (is_object($team)) {
                $item = BoincCreditTeam::lookup("teamid=$item->teamid and appid=$app->id");

                if (is_object($item)) {
                    $z                 = 0;
                    $store[$x][$y][$z] = $item->expavg;
                    $z++;
                    $store[$x][$y][$z] = $item->total;
                } else {
                    $z                 = 0;
                    $store[$x][$y][$z] = 0.0;
                    $z++;
                    $store[$x][$y][$z] = 0.0;
                }
                $y++;
            }
        }

        $x++;
    }
    return $store;
}

function retrieve_credit_user($data) {
    global $apps;
    $x = 1;
    $c = 0;

    foreach ($data as $item) {
        $user = BoincUser::lookup_id($item->userid);
        if (is_object($user)) {
            $sign         = user_links($user, BADGE_HEIGHT_MEDIUM);
            $store[0][$c] = $sign;
            $c++;
        }
    }

    foreach ($apps as $app) {
        $y = 0;

        foreach ($data as $item) {
            $user = BoincUser::lookup_id($item->userid);
            if (is_object($user)) {
                $item = BoincCreditUser::lookup("userid=$item->userid and appid=$app->id");
                if (is_object($item)) {
                    $z                 = 0;
                    $store[$x][$y][$z] = $item->expavg;
                    $z++;
                    $store[$x][$y][$z] = $item->total;
                } else {
                    $z                 = 0;
                    $store[$x][$y][$z] = 0.0;
                    $z++;
                    $store[$x][$y][$z] = 0.0;
                }
                $y++;
            }
        }
        $x++;
    }
    return $store;
}

function get_top_items($is_team, $appid, $is_total, $offset) {
    global $items_per_page;
    $x = $is_total ? "total" : "expavg";

    if ($is_team) {

        $data  = BoincCreditTeam::get_list("appid=$appid", $x, $offset . ", " . $items_per_page);
        $store = retrieve_credit_team($data);
    } else {
        $data  = BoincCreditUser::get_list("appid=$appid", $x, $offset . ", " . $items_per_page);
        $store = retrieve_credit_user($data);
    }

    return $store;
}

$is_team        = get_int('is_team', true);
$appid          = get_int('appid', true);
$is_total       = get_int('is_total', true);
$items_per_page = 20;
$offset         = get_int('offset', true);

if (!$offset)
    $offset = 0;
if ($offset % $items_per_page)
    $offset = 0;

$x = $is_team ? tra("Top teams by application") : tra("Top participants by application");
page_head($x);

$apps = BoincApp::enum("deprecated=0");
if (!$appid) {
    $appid = $apps[0]->id;
}

if ($offset < ITEM_LIMIT) {
    $cache_args = "appid=$appid&is_team=$is_team&is_total=$is_total&offset=$offset";
    $cacheddata = get_cached_data(TOP_PAGES_TTL, $cache_args);

    // Do we have the data in cache?
    //
    if ($cacheddata) {
        $data = unserialize($cacheddata); // use the cached data
    } else {
        //if not do queries etc to generate new data
        $data = get_top_items($is_team, $appid, $is_total, $offset);

        //save data in cache
        //
        set_cached_data(TOP_PAGES_TTL, serialize($data), $cache_args);
    }
} else {
    error_page(tra("Limit exceeded - Sorry, first %1 items only", ITEM_LIMIT));
}

start_table('table_striped');
show_header($is_team, $apps, $appid, $is_total);

$i = 1 + $offset;

//The number of columns is the number of items we currently have
//available to display in each row
//
for ($x = 0; $x < count($data[0]); $x++) {
    show_row($data, $x);
    $i++;
}

end_table();

if ($offset > 0) {
    $new_offset = $offset - $items_per_page;
    echo "<a href=per_app_list.php?appid=$appid&is_team=$is_team&is_total=$is_total&offset=$new_offset>" . tra("Previous %1", $items_per_page) . "</a> &middot; ";
}

if (sizeof($data[0]) == $items_per_page) {
    $new_offset = $offset + $items_per_page;
    echo "<a href=per_app_list.php?appid=$appid&is_team=$is_team&is_total=$is_total&offset=$new_offset>" . tra("Next %1", $items_per_page) . "</a>";
}

page_tail();

?>
