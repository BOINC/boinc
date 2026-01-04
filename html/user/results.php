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

// show recent results for a host or user

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/result.inc");

check_get_args(array("hostid", "userid", "offset", "appid", "state", "show_names"));

$config = get_config();
if (!parse_bool($config, "show_results")) {
    error_page(tra("This feature is turned off temporarily"));
}

// Add DataTables CSS and JS for client-side table sorting
$head_extra = '
<link rel="stylesheet" type="text/css" href="https://cdn.datatables.net/1.13.8/css/dataTables.bootstrap4.min.css">
<script type="text/javascript" src="https://cdn.datatables.net/1.13.8/js/jquery.dataTables.min.js"></script>
<script type="text/javascript" src="https://cdn.datatables.net/1.13.8/js/dataTables.bootstrap4.min.js"></script>
';

BoincDb::get(true);

$results_per_page = 20;

$hostid = get_int("hostid", true);
$userid = get_int("userid", true);
$offset = get_int("offset", true);
$appid = get_int("appid", true);
if (!$offset) $offset=0;
$state = get_int("state", true);
if (!$state) $state=0;
$show_names = get_int("show_names", true);
if (!$show_names) $show_names=0;

$s = $state_name[$state];
if ($appid) {
    $app = BoincApp::lookup_id($appid);
    if ($app) {
        $s .= " $app->user_friendly_name ";
    }
}

if ($hostid) {
    $host = BoincHost::lookup_id($hostid);
    if (!$host) error_page(tra("No computer with ID %1 found", $hostid));
    $clause = "hostid=$hostid";
    page_head(tra("$s tasks for computer %1", $host->id), null, false, "", $head_extra);
    $show_host_link = false;
} else if ($userid){
    $user = get_logged_in_user();
    if ($userid != $user->id) {
        error_page(tra("No access"));
    }
    $clause = "userid=$userid";
    page_head(tra("$s tasks for $user->name"), null, false, "", $head_extra);
    $show_host_link = true;
} else {
    error_page(tra("Missing user ID or host ID"));
}

$clause2 = $clause. $state_clause[$state];
if ($appid) {
    $clause2 .= ' AND appid='.$appid;
}

if ($show_names) {
    $order_clause = "order by name";
} else {
    $order_clause = "order by sent_time desc";
}
$query = "$clause2 $order_clause limit $offset,".($results_per_page+1);
$results = BoincResult::enum($query);

$info = new StdClass;
$info->number_of_results = count($results);
$info->clause = $clause;
$info->results_per_page = $results_per_page;
$info->offset = $offset;
$info->show_names = $show_names;
$info->state = $state;
$info->appid = $appid;

$nav = result_navigation($info, $clause);
$i = 0;
if (count($results)) {
    echo $nav;
    result_table_start(true, $show_host_link, $info);
    foreach ($results as $result) {
        if ($i++ >= $results_per_page) break;
        show_result_row($result, true, $show_host_link, $show_names);
    }
    end_table();
} else {
    start_table();
    row1(tra("No tasks to display"));
    end_table();
}

echo $nav;

// Initialize DataTables for client-side sorting
echo '
<script type="text/javascript">
jQuery(document).ready(function() {
    // Check if the results table exists
    if (jQuery("#results-table").length) {
        jQuery("#results-table").DataTable({
            "paging": false,        // Disable DataTables pagination (use existing navigation)
            "searching": false,     // Disable search box (not needed)
            "info": false,          // Disable info text (e.g., "Showing 1 to 10 of 57 entries")
            "order": []             // No default sort, keep database order
        });
    }
});
</script>
';

page_tail();
?>
