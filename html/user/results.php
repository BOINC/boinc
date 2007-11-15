<?php

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
$i = 0;
$query = "$clause order by id desc limit $offset,".($results_per_page+1);
$results = BoincResult::enum($query);
$number_of_results = count($results);
echo show_result_navigation(
    $clause, $number_of_results, $offset, $results_per_page
);
foreach ($results as $result) {
    if ($i >= $results_per_page) break;
    show_result_row($result, true, false, true);
    $i++;
}
echo "</table>\n";

echo show_result_navigation(
    $clause, $number_of_results, $offset, $results_per_page
);

page_tail();
?>
