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

$config = get_config();
if (!parse_bool($config, "show_results")) {
    error_page(tra("This feature is turned off temporarily"));
}

BoincDb::get(true);

$results_per_page = 20;

$hostid = get_int("hostid", true);
$userid = get_int("userid", true);
$offset = get_int("offset", true);
if (!$offset) $offset=0;
$state = get_int("state", true);
if (!$state) $state=0;
$show_names = get_int("show_names", true);
if (!$show_names) $show_names=0;

$s = $state_name[$state];
if ($hostid) {
    $host = BoincHost::lookup_id($hostid);
    if (!$host) error_page(tra("No computer with ID %1 found", $hostid));
    $clause = "hostid=$hostid";
    page_head(tra("$s tasks for computer %1", $host->id));
} else if ($userid){
    $user = get_logged_in_user();
    if ($userid != $user->id) {
        error_page(tra("No access"));
    }
    $clause = "userid=$userid";
    page_head(tra("$s tasks for $user->name"));
} else {
    error_page(tra("Missing user ID or host ID"));
}

$clause2 = $clause. $state_clause[$state];

$query = "$clause2 order by id desc limit $offset,".($results_per_page+1);
$results = BoincResult::enum($query);

$info = null;
$info->number_of_results = count($results);
$info->clause = $clause;
$info->results_per_page = $results_per_page;
$info->offset = $offset;
$info->show_names = $show_names;
$info->state = $state;

if (count($results)) {
    echo show_result_navigation($info);
    result_table_start(true, false, $info);
    $i = 0;
    foreach ($results as $result) {
        if ($i >= $results_per_page) break;
        show_result_row($result, true, false, $show_names, $i);
        $i++;
    }
    echo "</table>\n";
} else {
    start_table();
    row1(tra("No tasks to display"));
    end_table();
}

echo show_result_navigation($info);

page_tail();
?>
