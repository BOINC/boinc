<?php

require_once("../inc/db.inc");
require_once("../inc/xml.inc");

$nonce = process_user_text($_GET["nonce"]);

xml_header();

function reply($x) {
    echo "<am_query_reply>
    $x
</am_query_reply>
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
$tuser = lookup_tentative_user($nonce);

if (!$tuser) {
    error("nonce not found");
}

if (!$tuser->confirmed) {
    success("<confirmed>0</confirmed>");
}

$user = lookup_user_email_addr($tuser->email_addr);

if (!$user) {
    $authenticator = random_string();
    $cross_project_id = random_string();
    $now = time();
    $query = "insert into user (create_time, email_addr, authenticator, cross_project_id) values($now, '$tuser->email_addr', '$authenticator', '$cross_project_id')";
    $result = mysql_query($query);
    $user = lookup_user_auth($authenticator);
}

if (!$user) {
    error("couldn't create user record");
}
success("<account_key>$user->authenticator</account_key>");

?>
