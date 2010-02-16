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
require_once("../inc/user.inc");
require_once("../inc/boinc_db.inc");

$config = get_config();
$users_per_page = parse_config($config, "<users_per_page>");
if (!$users_per_page) {
    $users_per_page = 20;
}
define ('ITEM_LIMIT', 10000);

function get_top_participants($offset, $sort_by) {
    global $users_per_page;
    $db = BoincDb::get(true);
    if ($sort_by == "total_credit") {
        $sort_order = "total_credit desc";
    } else {
        $sort_order = "expavg_credit desc";
    }
    return BoincUser::enum(null, "order by $sort_order limit $offset,$users_per_page");
}

function user_table_start($sort_by) {
    start_table();
    echo "
        <tr>
        <th>".tra("Rank")."</th>
        <th>".tra("Name")."</th>
    ";
    if ($sort_by == "total_credit") {
        echo "
            <th><a href=top_users.php?sort_by=expavg_credit>".tra("Recent average credit")."</a></th>
            <th>".tra("Total credit")."</th>
        ";
    } else {
        echo "
            <th>".tra("Recent average credit")."</th>
            <th><a href=top_users.php?sort_by=total_credit>".tra("Total credit")."</a></th>
        ";
    }
    echo "
        <th>".tra("Country")."</th>
        <th>".tra("Participant since")."</th>
        </tr>
    ";
}

function show_user_row($user, $i) {
    echo "
        <tr class=row1>
        <td>$i</td>
        <td>", user_links($user), "</td>
        <td align=right>", format_credit_large($user->expavg_credit), "</td>
        <td align=right>", format_credit_large($user->total_credit), "</td>
        <td>", $user->country, "</td>
        <td>", time_str($user->create_time),"</td>
        </tr>
    ";
}

$sort_by = get_str("sort_by", true);
switch ($sort_by) {
case "total_credit":
case "expavg_credit":
    break;
default:
    $sort_by = "expavg_credit";
}

$offset = get_int("offset", true);
if (!$offset) $offset=0;
if ($offset % $users_per_page) $offset = 0;

if ($offset < ITEM_LIMIT) {
    $cache_args = "sort_by=$sort_by&offset=$offset";
    $cacheddata=get_cached_data(TOP_PAGES_TTL,$cache_args);

    // Do we have the data in cache?
    //
    if ($cacheddata){
        $data = unserialize($cacheddata); // use the cached data
    } else {
        //if not do queries etc to generate new data
        $data = get_top_participants($offset, $sort_by);

        //save data in cache
        //
        set_cache_data(serialize($data),$cache_args);
    }
} else {
    error_page(tra("Limit exceeded - Sorry, first %1 items only", ITEM_LIMIT));
}

// Now display what we've got (either gotten from cache or from DB)
page_head(tra("Top participants"));
user_table_start($sort_by);
$i = 1 + $offset;
$n = sizeof($data);
foreach ($data as $user) {
    show_user_row($user, $i);
    $i++;
}
echo "</table>\n<p>";
if ($offset > 0) {
    $new_offset = $offset - $users_per_page;
    echo "<a href=top_users.php?sort_by=$sort_by&amp;offset=$new_offset>".tra("Previous %1", $users_per_page)."</a> | ";

}
if ($n==$users_per_page){ //If we aren't on the last page
    $new_offset = $offset + $users_per_page;
    echo "<a href=top_users.php?sort_by=$sort_by&amp;offset=$new_offset>".tra("Next %1", $users_per_page)."</a>";
}

page_tail();

?>
