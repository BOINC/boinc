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
require_once("../inc/host.inc");
require_once("../inc/boinc_db.inc");

check_get_args(array("sort_by", "offset"));

$config = get_config();
$hosts_per_page = parse_config($config, "<hosts_per_page>");
if (!$hosts_per_page) {
    $hosts_per_page = 20;
}
define ('ITEM_LIMIT', 10000);

function get_top_hosts($offset, $sort_by) {
    global $hosts_per_page;
    $db = BoincDb::get(true);
    if ($sort_by == "total_credit") {
        $sort_order = "total_credit desc";
    } else {
        $sort_order = "expavg_credit desc";
    }
    return BoincHost::enum(null, "order by $sort_order limit $offset, $hosts_per_page");
}

$sort_by = get_str('sort_by', true);
if ($sort_by) {
    sanitize_sort_by($sort_by);
} else {
    $sort_by = 'expavg_credit';
}

$offset = get_int("offset", true);
if (!$offset) $offset=0;
if ($offset % $hosts_per_page) $offset = 0;

if ($offset >= ITEM_LIMIT) {
    error_page(tra("Limit exceeded - Sorry, first %1 items only", ITEM_LIMIT));
}

$cache_args = "sort_by=$sort_by&offset=$offset";
$cacheddata = get_cached_data(TOP_PAGES_TTL, $cache_args);
if ($cacheddata){
    $data = unserialize($cacheddata);
} else {
    $data = get_top_hosts($offset,$sort_by);
    set_cached_data(TOP_PAGES_TTL, serialize($data), $cache_args);
};


// Now display what we've got (either gotten from cache or from DB)
//
page_head(tra("Top computers"));
top_host_table_start($sort_by);
$i = 1 + $offset;
$n = sizeof($data);
foreach($data as $host) {
    show_host_row($host, $i, false, true, false);
    $i++;
}
end_table();

if ($offset > 0) {
    $new_offset = $offset - $hosts_per_page;
    echo "<a href=top_hosts.php?sort_by=$sort_by&amp;offset=$new_offset>".tra("Previous %1", $hosts_per_page)."</a> &middot; ";

}

if ($n==$hosts_per_page){ //If we aren't on the last page
    $new_offset = $offset + $hosts_per_page;
    echo "<a href=top_hosts.php?sort_by=$sort_by&amp;offset=$new_offset>".tra("Next %1", $hosts_per_page)."</a>";
}

page_tail();


?>
