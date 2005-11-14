<?php

require_once("../inc/db.inc");
require_once("../inc/xml.inc");

function reply($x) {
    echo "<am_set_host_info_reply>
    $x
</am_set_host_info_reply>
";
    exit();
}

function error($x) {
    reply("<error>$x</error>");
}

function success($x) {
    reply("<success/>\n$x");
}

db_init();

xml_header();

$auth = process_user_text($_GET["account_key"]);
$user = lookup_user_auth($auth);
if (!$user) {
    error("no such user");
}

$hostid = get_int("hostid");

$host = lookup_host($hostid);
if (!$host || $host->userid != $user->id) {
    error("no such host");
}

$venue = process_user_text($_GET["venue"]);

$result = mysql_query("update host set venue='$venue' where id=$hostid");
if ($result) {
    success("");
} else {
    error("database error");
}

?>
