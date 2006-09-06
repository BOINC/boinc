<?php

require_once("../inc/db.inc");
require_once("../inc/xml.inc");

xml_header();

$retval = db_init_xml();
if ($retval) xml_error($retval);


$auth = process_user_text($_GET["account_key"]);
$user = lookup_user_auth($auth);
if (!$user) {
    xml_error(-136);
}

$hostid = get_int("hostid");

$host = lookup_host($hostid);
if (!$host || $host->userid != $user->id) {
    xml_error(-136);
}

$venue = process_user_text($_GET["venue"]);

$result = mysql_query("update host set venue='$venue' where id=$hostid");
if ($result) {
    echo "<am_set_host_info_reply>
    <success/>
</am_set_host_info_reply>
";
} else {
    xml_error(-1, "database error");
}

?>
