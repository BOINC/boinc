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

function merge_hosts($old_host, $new_host) {
   if ($old_host->id == $new_host->id) {
       fail("same host");
   }
   if (!hosts_compatible($old_host, $new_host)) {
       fail("Can't merge hosts - they're incompatible");
   }

   echo "<br>Merging $old_host->id into $new_host->id\n";

   // update the database:
   // - add credit from old to new host
   // - change results to refer to new host
   // - delete old host
   //
   $total_credit = $old_host->total_credit + $new_host->total_credit;
   $recent_credit = $old_host->expavg_credit + $new_host->expavg_credit;
   $result = mysql_query("update host set total_credit=$total_credit, expavg_credit=$recent_credit where id=$new_host->id");
   if (!$result) {
       fail("Couldn't update credit of new host");
   }
   $result = mysql_query("update result set hostid=$new_host->id where hostid=$old_host->id");
   if (!$result) {
       fail("Couldn't update results");
   }
   $result = mysql_query("delete from host where id=$old_host->id");
   if (!$result) {
       fail("Couldn't delete record of computer");
   }
}

    db_init();
    $user = get_logged_in_user();

    page_head("Merge computer records");

    $nhosts = $_GET["nhosts"];
    $hostid = $_GET["id_0"];
    $latest_host = get_host($hostid, $user);
    for ($i=1; $i<$nhosts; $i++) {
        $var = "id_$i";
        $hostid = $_GET[$var];
        if (!$hostid) continue;
        $host = get_host($hostid, $user);
        if ($host->create_time > $latest_host->create_time) {
            merge_hosts($latest_host, $host);
            $latest_host = $host;
        } else {
            merge_hosts($host, $latest_host);
        }
    }
    echo "
        <p><a href=hosts_user.php>Return to list of your computers</a>
    ";
    page_tail();

    //Header("Location: show_host_detail.php?hostid=$latest_host->id");

?>
