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

require_once("../inc/cache.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");
require_once("../inc/db.inc");

$config = get_config();
$teams_per_page = parse_config($config, "<teams_per_page>");
if (!$teams_per_page) {
        $teams_per_page = 20;
}
define('ITEM_LIMIT', 10000);

function get_top_teams($offset, $sort_by, $type){
    global $teams_per_page;
    $db = BoincDb::get(true);
    $type_clause = null;
    if ($type){
        $type_clause = "type=$type";
    }
    if ($sort_by == "total_credit") {
        $sort_order = "total_credit desc";
    } else {
        $sort_order = "expavg_credit desc";
    }
    return BoincTeam::enum($type_clause, "order by $sort_order limit $offset, $teams_per_page");
}

// These converter functions are here in case we later decide to use something 
// else than serializing to save temp data
//
function teams_to_store($participants){
    return serialize($participants);
}
function store_to_teams($data){
    return unserialize($data);
}

$sort_by = get_str("sort_by", true);
switch ($sort_by) {
case "total_credit":
case "expavg_credit":
    break;
default:
    $sort_by = "expavg_credit";
}

$type = get_int("type", true);
if ($type < 1 || $type > 7) {
    $type = 0;
}
$type_url="";
$type_name="";
if ($type) {
    $type_url = "&type=$type";
    $type_name = team_type_name($type);
}

$offset = get_int("offset", true);
if (!$offset) $offset=0;
if ($offset % $teams_per_page) $offset = 0;

if ($offset < ITEM_LIMIT) {
    $cache_args = "sort_by=$sort_by&offset=$offset$type_url";
    $cacheddata = get_cached_data(TOP_PAGES_TTL,$cache_args);
    //If we have got the data in cache
    if ($cacheddata){
        $data = store_to_teams($cacheddata); // use the cached data
    } else {
        //if not do queries etc to generate new data
        $data = get_top_teams($offset,$sort_by,$type);
        
        // Calculate nusers before storing into the cache
        foreach ($data as $team) {
            $team->nusers = team_count_members($team->id);
        }
        //save data in cache
        set_cache_data(teams_to_store($data),$cache_args);
    }
} else {
    error_page(tra("Limit exceeded - Sorry, first %1 items only", ITEM_LIMIT));
}


// Now display what we've got (either gotten from cache or from DB)
page_head(tra("Top %1 teams", $type_name));

if (count($data) == 0) {
    echo tra("There are no %1 teams", $type_name);
} else {
    start_table();
    team_table_start($sort_by, $type_url);
    $i = 1 + $offset;
    $n = sizeof($data);
    foreach ($data as $team) {
        show_team_row($team, $i);
        $i++;
    }
    echo "</table>\n<p>";
    if ($offset > 0) {
        $new_offset = $offset - $teams_per_page;
        echo "<a href=top_teams.php?sort_by=$sort_by&amp;offset=$new_offset".$type_url.">".tra("Previous %1", $teams_per_page)."</a> | ";

    }
    if ($n==$teams_per_page){ //If we aren't on the last page
        $new_offset = $offset + $teams_per_page;
        echo "<a href=top_teams.php?sort_by=$sort_by&amp;offset=$new_offset".$type_url.">".tra("Next %1", $teams_per_page)."</a>";
    }
}
page_tail();

?>
