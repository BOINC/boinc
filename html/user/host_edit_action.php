<?php
    require_once("util.inc");
    require_once("host.inc");

    db_init();
    $user = get_logged_in_user();

    $hostid = $_GET["hostid"];
    $targetid = $_GET["targetid"];

    $result = mysql_query("select * from host where id=$hostid");
    $old_host = mysql_fetch_object($result);
    mysql_free_result($result);
    if (!$old_host || $old_host->userid != $user->id) {
        echo "Host not found";
        exit();
    }
    $result = mysql_query("select * from host where id=$targetid");
    $new_host = mysql_fetch_object($result);
    mysql_free_result($result);
    if (!$new_host || $new_host->userid != $user->id) {
        echo "Host not found";
        exit();
    }

    if (!hosts_compatible($old_host, $new_host)) {
        echo "Can't merge hosts";
        exit();
    }

    // update the database:
    // - add credit from old to new host
    // - change results to refer to new host
    // - delete old host

    $t = $old_host->total_credit + $new_host->total_credit;
    $a = $old_host->expavg_credit + $new_host->expavg_credit;
    $result = mysql_query("update host set total_credit=$t, expavg_credit=$a where id=$new_host->id");
    if ($result) {
        $result = mysql_query("update result set hostid=$targetid where hostid=$hostid");
        if ($result) {
            if ($hostid != $targetid) {
                $result = mysql_query("delete from host where id=$hostid");
            }
        }
    }
    if ($result) {
        Header("Location: show_host_detail.php?hostid=$targetid&private=1");
    } else {
        page_head("Host merge failed");
        echo "Couldn't update database - please try again later";
        page_tail();
    }

?>
