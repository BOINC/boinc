<?php

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/user.inc");
require_once("../inc/host.inc");

$hostid = get_int("hostid");
$ipprivate = get_str("ipprivate", true);
$host = BoincHost::lookup_id($hostid);
if (!$host) {
    echo "Couldn't find computer";
    exit();
}

$user = get_logged_in_user(false);
if ($user->id != $host->userid) {
    $user = null;
}

page_head("Computer summary");
show_host($host, $user, $ipprivate);
page_tail();

?>
