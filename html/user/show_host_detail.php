<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/user.inc");
require_once("../inc/host.inc");

db_init();
$hostid = get_int("hostid");
$ipprivate = $_GET["ipprivate"];
$host = lookup_host($hostid);
if (!$host) {
    echo "Couldn't find computer";
    exit();
}
$private = false;
$user = get_logged_in_user(false);
if ($user && $user->id == $host->userid) {
    $private = true;
}

page_head("Computer summary");
show_host($host, $private, $ipprivate);
page_tail();

?>
