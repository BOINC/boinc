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

$hostid = get_int("hostid");
host_update_credit($hostid);
echo "<br>Host credit updated";
page_tail();

?>
