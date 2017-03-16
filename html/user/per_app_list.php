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

check_get_args(array("is_team", "appid", "is_total", "offset"));

define ('ITEM_LIMIT', 10000);

// return a column title (Average or Total),
// hyperlinked if this is not the current sort column
//
function col_title($is_team, $app, $appid, $is_total, $i) {
    $x = $i?"Total":"Average";
    if ($app->id == $appid && ($is_total?$i:!$i)) {
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
        echo "<th colspan=2>$app->name</th>\n";
    }
    echo "</tr>";

    echo "<tr>";
    echo "<th>Rank</th><th>Name</th>\n";
    foreach ($apps as $app) {
        for ($i=0; $i<2; $i++) {
            $x = col_title($is_team, $app, $appid, $is_total, $i);
            echo "<th>$x</th>\n";
        }
    }
    echo "</tr>";

}

// show a user or team, with their credit for each app
//
function show_row($item, $apps, $is_team, $i) {
    global $i;
    if ($is_team) {
        $team = BoincTeam::lookup_id($item->teamid);
        if (!$team) {
            $i--;
            return;
        }
        $x = "<td>".team_links($team)."</td>\n";
    } else {
        $user = BoincUser::lookup_id($item->userid);
        if (!$user) {
            $i--;
            return;
        }
        $x= "<td>".user_links($user, BADGE_HEIGHT_MEDIUM)."</td>\n";
    }
    echo "<tr>";
    echo "<td>$i</td>\n";
    echo $x;

    foreach ($apps as $app) {
        if ($app->id == $item->appid) {
            $c = $item;
        } else {
            if ($is_team) {
                $c = BoincCreditTeam::lookup("teamid=$item->teamid and appid=$app->id");
            } else {
                $c = BoincCreditUser::lookup("userid=$item->userid and appid=$app->id");
            }
            if (!$c) {
                $c = new StdClass;
                $c->expavg = 0;
                $c->total = 0;
            }
        }
        echo "<td align=right>".format_credit($c->expavg)."</td><td align=right>".format_credit_large($c->total)."</td>\n";
    }
    echo "</tr>\n";
}

function get_top_items($is_team, $appid, $is_total, $offset) {
    global $items_per_page;
    $x = $is_total?"total":"expavg";
    if ($is_team) {
        return  BoincCreditTeam::enum("appid=$appid order by $x desc limit $offset,$items_per_page");
    } else {
        return  BoincCreditUser::enum("appid=$appid order by $x desc limit $offset,$items_per_page");
    }
}

$is_team = get_int('is_team', true);
$appid = get_int('appid', true);
$is_total = get_int('is_total', true);
$items_per_page = 20;
$offset = get_int('offset', true);
if (!$offset) $offset=0;
if ($offset % $items_per_page) $offset = 0;
$x = $is_team?"teams":"participants";
page_head(tra("Top %1 by application", $x));
$apps = BoincApp::enum("deprecated=0");
if (!$appid) {
    $appid = $apps[0]->id;
}

if ($offset < ITEM_LIMIT) {
     $cache_args = "appid=$appid&is_team=$is_team&is_total=$is_total&offset=$offset";
     $cacheddata = get_cached_data(TOP_PAGES_TTL,$cache_args);

     // Do we have the data in cache?
     //
     if ($cacheddata){
         $data = unserialize($cacheddata); // use the cached data
     } else {
         //if not do queries etc to generate new data
         $data = get_top_items($is_team, $appid, $is_total, $offset);

         //save data in cache
         //
         set_cached_data(TOP_PAGES_TTL, serialize($data),$cache_args);
       }
} else {
    error_page(tra("Limit exceeded - Sorry, first %1 items only", ITEM_LIMIT));
  }

start_table('table_striped');
show_header($is_team, $apps, $appid, $is_total);
$i = 1 + $offset;
$n = sizeof($data);

foreach ($data as $item) {
         show_row($item, $apps, $is_team, $i);
         $i++;
}

end_table();

if ($offset > 0) {
    $new_offset = $offset - $items_per_page;
    echo "<a href=per_app_list.php?appid=$appid&is_team=$is_team&is_total=$is_total&offset=$new_offset>".tra("Previous %1", $items_per_page)."</a> &middot; ";
}

if ($n == $items_per_page) {
    $new_offset = $offset + $items_per_page;
    echo "<a href=per_app_list.php?appid=$appid&is_team=$is_team&is_total=$is_total&offset=$new_offset>".tra("Next %1", $items_per_page)."</a>";
}

page_tail();

?>
