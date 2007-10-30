<?php

require_once("../inc/boinc_db.inc");
require_once("../inc/xml.inc");

xml_header();

$db = BoincDb::get();
if (!$db) xml_error($retval);


$auth = process_user_text($_GET["account_key"]);
$user = BoincUser::lookup("authenticator='$auth'");
if (!$user) {
    xml_error(-136);
}

$hostid = get_int("hostid");

$host = BoincHost::lookup_id($hostid);
if (!$host || $host->userid != $user->id) {
    xml_error(-136);
}

$venue = process_user_text($_GET["venue"]);

$result = $host->update("venue='$venue'");
if ($result) {
    echo "<am_set_host_info_reply>
    <success/>
</am_set_host_info_reply>
";
} else {
    xml_error(-1, "database error");
}

?>
