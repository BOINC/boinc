<?php
    require_once("../inc/db.inc");
    require_once("../inc/util.inc");
    require_once("../inc/host.inc");

function fail($msg) {
    echo "Error: $msg";
    page_tail();
    exit();
}

function get_host($hostid, $user) {
    $host = lookup_host($hostid);
    if (!$host || $host->userid != $user->id) {
        fail("We have no record of that computer");
    }
    return $host;
}

    db_init();
    $user = get_logged_in_user();

    page_head("Updating computer credit");

    $hostid = $_GET["hostid"];
    $latest_host = get_host($hostid, $user);
    $r = mysql_query("select * from result where hostid=$hostid");
    $t = 0;
    $a = 0;
    $hl = 86400*7;
    $now = time();
    while ($result = mysql_fetch_object($r)) {
        $t += $result->granted_credit;
        $td = $now - $result->received_time;
        $a += $result->granted_credit*exp(-$td*M_LN2/$hl);
    }
    $a /= 2;    // not sure why this is needed
    mysql_free_result($r);
    mysql_query("update host set total_credit=$t, expavg_credit=$a where id=$hostid");
    echo "Host credit updated";
    page_tail();

?>
