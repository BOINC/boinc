<?php
    require_once("db.inc");
    require_once("util.inc");
    require_once("host.inc");

function fail($msg) {
    page_head("Host merge failed");
    echo $msg;
    page_tail();
    exit();
}

    db_init();
    $user = get_logged_in_user();

    $hostid = $_GET["hostid"];
    $targetid = $_GET["targetid"];

    if ($hostid == $targetid) {
        fail("same host");
    }

    $result = mysql_query("select * from host where id=$hostid");
    $old_host = mysql_fetch_object($result);
    mysql_free_result($result);
    if (!$old_host || $old_host->userid != $user->id) {
        fail("Host not found");
    }

    $result = mysql_query("select * from host where id=$targetid");
    $new_host = mysql_fetch_object($result);
    mysql_free_result($result);
    if (!$new_host || $new_host->userid != $user->id) {
        fail("Host not found");
    }

    if (!hosts_compatible($old_host, $new_host)) {
        fail("Can't merge hosts - they're incompatible");
    }

    // update the database:
    // - add credit from old to new host
    // - change results to refer to new host
    // - delete old host

    $t = $old_host->total_credit + $new_host->total_credit;
    $a = $old_host->expavg_credit + $new_host->expavg_credit;
    $result = mysql_query("update host set total_credit=$t, expavg_credit=$a where id=$new_host->id");
    if (!result) {
        fail("Couldn't update credit of new host");
    }
    $result = mysql_query("update result set hostid=$targetid where hostid=$hostid");
    if (!$result) {
        fail("Couldn't update results");
    }
    $result = mysql_query("delete from host where id=$hostid");
    if (!$result) {
        fail("Couldn't delete host");
    }
    Header("Location: show_host_detail.php?hostid=$targetid&private=1");

?>
