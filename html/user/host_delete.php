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

page_head("Delete record of computer");

$hostid = get_int("hostid");
$host = get_host($hostid, $user);
if (host_nresults($host)==0) {
    mysql_query("delete from host where id=$hostid");
} else {
    fail("existing results");
}
echo "
    Host deleted.
    <p><a href=hosts_user.php>Return to list of your computers</a>
";
page_tail();

?>
