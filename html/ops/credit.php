<?php

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

require_once("../inc/boinc_db.inc");

define('COBB_SCALE', 200/86400e9);


function show_result($r) {
    $hav = BoincHostAppVersion::lookup($r->hostid, $r->app_version_id);
    $av = BoincAppVersion::lookup_id($r->app_version_id);
    $raw_credit = $r->elapsed_time*$r->flops_estimate*COBB_SCALE;
    echo "<hr><pre>
Result ID: <a href=db_action.php?table=result&id=$r->id>$r->id</a>
Host: <a href=credit.php?hostid=$r->hostid&avid=$r->app_version_id>$r->hostid</a>
elapsed_time: $r->elapsed_time
flops_estimate: ".$r->flops_estimate/1e9."
app_version_id: $r->app_version_id
";
    if ($av) {
        $host_scale = $av->pfc_avg/$hav->pfc_avg;
        $av_scale = $av->pfc_scale;
        echo "app version scale: $av->pfc_scale\n";
    } else {
        $host_scale = 1;
        $av_scale = 1;
    }
    echo "host scale: $host_scale
Credit:
    Original: $r->claimed_credit
    Raw: $raw_credit
    Scaled: ".$raw_credit*$host_scale*$av_scale."
    Granted: $r->granted_credit
";

    echo "Host app version:
    PFC avg: $hav->pfc_avg
    host scale: ".$av->pfc_avg/$hav->pfc_avg ."
";
}

function show_av($avid) {
    $av = BoincAppVersion::lookup_id($avid);
    if (!$av) {
        echo "not found";
        return;
    }
    echo "<hr>
App version ID: $avid
plan class: $av->plan_class
PFC: ".$av->pfc_avg."
scale: ".$av->pfc_scale."
";
}

function handle_result($resultid) {
    $r = BoincResult::lookup_id($resultid);
    $w = BoincWorkunit::lookup_id($r->workunitid);
    $rs = BoincResult::enum("workunitid=$r->workunitid and validate_state=1");
    $app_version_ids = array();
    foreach ($rs as $r) {
        show_result($r);
        $app_version_ids[] = $r->app_version_id;
    }
    $app_version_ids = array_unique($app_version_ids);
    foreach ($app_version_ids as $avid) {
        show_av($avid);
    }
}

function host_history($hostid, $avid) {
    $rs = BoincResult::enum("hostid=$hostid and app_version_id=$avid and validate_state=1 order by id desc limit 100");
    echo "<pre>Results for host $hostid\n";
    $n = 0;
    $total=0;
    foreach($rs as $r) {
        $raw_credit = $r->elapsed_time*$r->flops_estimate*COBB_SCALE;
        $n++;
        $total += $raw_credit;
        echo "ID <a href=credit.php?resultid=$r->id>$r->id</a> raw $raw_credit
";
    }
    $avg = $total/$n;
    echo "avg: $avg\n";
    echo "</pre>\n";
}

$resultid = $_GET['resultid'];

if ($resultid) {
    handle_result($resultid);
    exit;
}

$hostid = $_GET['hostid'];
$avid = $_GET['avid'];
if ($hostid && $avid) {
    host_history($hostid, $avid);
    exit;
}

echo "??";

?>
