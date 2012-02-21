<?php

// generate a data file for use by credit_test.php

require_once("../inc/boinc_db.inc");

$db = BoincDb::get();

$min_claimed_credit = 0;
$max_claimed_credit = 5000;
$limit = 0;
$appid = 2;

$query = "select result.id, result.workunitid, result.appid, result.hostid, result.claimed_credit, result.app_version_id, result.elapsed_time, result.flops_estimate, result.cpu_time, workunit.rsc_fpops_est from result, workunit where workunit.id = result.workunitid and result.validate_state=1 and claimed_credit > $min_claimed_credit and claimed_credit < $max_claimed_credit and app_version_id<>0";

if ($appid) {
    $query .= " and result.appid = $appid";
}
if ($limit) {
    $query .= " limit $limit";
}

$r = $db->do_query($query);
$f = fopen("credit_test_data", "w");
while ($x = mysql_fetch_object($r)) {
    fprintf($f, "$x->id $x->workunitid $x->appid $x->hostid $x->claimed_credit $x->app_version_id $x->elapsed_time $x->flops_estimate $x->cpu_time $x->rsc_fpops_est\n");
}
fclose($f);

?>
