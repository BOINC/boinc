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
		$error = merge_hosts($latest_host, $host);
        if ($error) {
            fail($error);
        }
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
