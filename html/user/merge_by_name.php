<?php

require_once("../inc/db.inc");
require_once("../inc/host.inc");

db_init();

function merge_name($list) {
    // find the newest one
    //
    $newest_host = $list[0];
    echo "<br><br>Processing $newest_host->domain_name\n";
    foreach ($list as $host) {
        if ($host->create_time > $newest_host->create_time) {
            $newest_host = $host;
        }
    }
    foreach ($list as $host) {
        if ($host->id == $newest_host->id) {
            continue;
        }
        $error = merge_hosts($host, $newest_host);
        if (!$error) {
            echo "<b>merged $host->id into $newest_host->id\n";
        } else {
            echo "<br>$error\n";
        }
    }
}

function merge_by_name($userid) {
    $hosts = array();
    $result = mysql_query("select * from host where userid=$userid");
    while ($host = mysql_fetch_object($result)) {
        $hosts[$host->domain_name][] = $host;
    }
    foreach($hosts as $hlist) {
        merge_name($hlist);
    }
}

$user = get_logged_in_user();

page_head("Merge computers by name");

if ($_GET['confirmed']) {
    merge_by_name($user->id);
    echo "
        <p><a href=hosts_user.php>
        Return to the list of your computers</a>.
    ";
} else {
    echo "
        This operation will merge all of your computers
        that have the same domain name.
        <p>
        For each name, it will merge all older computers
        having that name with the newest computer having that name.
        Incompatible computers will not be merged.
        <p>
        Click <a href=merge_by_name.php?confirmed=1>here</a>
        if you're sure you want to do this.
        <p>Click <a href=hosts_user.php>here</a>
        to return to the list of your computers.
    ";
}
page_tail();
?>
