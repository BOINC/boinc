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
    error_page("This feature is turned off temporarily");
}

$results_per_page = 20;

$hostid = get_int("hostid", true);
$userid = get_int("userid", true);
$offset = get_int("offset", true);
if (!$offset) $offset=0;

if ($hostid) {
    $host = BoincHost::lookup_id($hostid);
    $type = "computer";
    $clause = "hostid=$hostid";
} else {
    $user = get_logged_in_user();
    if ($userid != $user->id) {
        error_page("No access");
    }
    $type = "user";
    $clause = "userid=$userid";
}
page_head("Tasks for $type");
result_table_start(true, false, true);
$query = "$clause order by id desc limit $offset,".($results_per_page+1);
$results = BoincResult::enum($query);
$number_of_results = count($results);
echo show_result_navigation(
    $clause, $number_of_results, $offset, $results_per_page
);
$i = 0;
foreach ($results as $result) {
    if ($i >= $results_per_page) break;
    show_result_row($result, true, false, true, $i);
    $i++;
}
echo "</table>\n";

echo show_result_navigation(
    $clause, $number_of_results, $offset, $results_per_page
);

page_tail();
?>
