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
    $r = mysql_query("select * from result where hostid=$hostid order by received_time");
    $t = 0;
    $avg = 0;
    $hl = 86400*7;
    $now = time();
    $avg_time = 0;
    $half_life = 86400*7;
    while ($result = mysql_fetch_object($r)) {
        if ($result->granted_credit <= 0) continue;
        $t += $result->granted_credit;

        // the following taken from lib/util.C
        //
        if ($avg_time) {
            $diff = $result->received_time - $avg_time;
            if ($diff <=0) $diff = 3600;
            $diff_days = $diff/86400;
            $weight = exp(-$diff*M_LN2/$half_life);
            $avg *= $weight;
            $avg += (1-$weight)*($result->granted_credit/$diff_days);
        } else {
            $dd = ($result->received_time - $result->sent_time)/86400;
            $avg = $result->granted_credit/$dd;
        }
        $avg_time = $result->received_time;
        echo "<br>$avg\n";
    }
    mysql_free_result($r);

    $diff = $now - $avg_time;
    $weight = exp(-$diff*M_LN2/$half_life);
    $avg *= $weight;
    mysql_query("update host set total_credit=$t, expavg_credit=$avg where id=$hostid");
    echo "<br>Host credit updated";
    page_tail();

?>
