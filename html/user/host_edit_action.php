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

// invariant: old_host.create_time < new_host.create_time
//
function merge_hosts($old_host, $new_host) {
    if ($old_host->id == $new_host->id) {
        fail("same host");
    }
    if (!hosts_compatible($old_host, $new_host)) {
        fail("<br>Can't merge host $old_host->id into $new_host->id - they're incompatible");
    }

    echo "<br>Merging host $old_host->id into host $new_host->id\n";

	// decay the average credit of both hosts
	//
	$now = time();
	update_average($now, 0, 0, $old_host->expavg_credit, $old_host->expavg_time);
	update_average($now, 0, 0, $new_host->expavg_credit, $new_host->expavg_time);

    // update the database:
    // - add credit from old to new host
    // - change results to refer to new host
    // - put old host in "zombie" state
    // - update rpc_seqno if needed
    //
    $total_credit = $old_host->total_credit + $new_host->total_credit;
    $recent_credit = $old_host->expavg_credit + $new_host->expavg_credit;
    $result = mysql_query("update host set total_credit=$total_credit, expavg_credit=$recent_credit, expavg_time=$now where id=$new_host->id");
    if (!$result) {
        fail("Couldn't update credit of new computer");
    }
    $result = mysql_query("update result set hostid=$new_host->id where hostid=$old_host->id");
    if (!$result) {
        fail("Couldn't update results");
    }

    $result = mysql_query("update host set total_credit=0, expavg_credit=0, userid=0, rpc_seqno=$new_host->id where id=$old_host->id");
    if (!$result) {
        fail("Couldn't retire old computer");
    }
    echo "<br>Retired old computer $old_host->id\n";
}

db_init();
$user = get_logged_in_user();

page_head("Merge computer records");

$nhosts = get_int("nhosts");
$hostid = get_int("id_0");
$latest_host = get_host($hostid, $user);
for ($i=1; $i<$nhosts; $i++) {
	$var = "id_$i";
	$hostid = get_int($var, true);
    if (!$hostid) continue;
	$host = get_host($hostid, $user);
	if ($host->create_time > $latest_host->create_time) {
		merge_hosts($latest_host, $host);
		$latest_host = $host;
	} else {
		merge_hosts($host, $latest_host);
	}
	// reread latest_host from database since we just
	// updated its credits
	//
	$latest_host = lookup_host($latest_host->id);
}
echo "
	<p><a href=hosts_user.php>Return to list of your computers</a>
";
page_tail();

//Header("Location: show_host_detail.php?hostid=$latest_host->id");

?>
