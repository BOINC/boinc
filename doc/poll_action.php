<?php
require_once("docutil.php");
require_once("poll.inc");
require_once("poll_data.inc");

function error() {
    page_head(tr(POLL_ERROR_TITLE));
    echo tr(POLL_ERROR_TEXT);
    page_tail();
    exit();
}

mysql_pconnect("localhost", "boincadm", null);
mysql_select_db("poll");

session_set_cookie_params(86400*365);
session_start();
$uid = session_id();

$x = parse_form();
$xml = generate_xml($x);

$response = select_response($uid);
if ($response) {
    $result = update_response($uid, $xml);
} else {
    $result = new_response($uid, $xml);
}
if ($result) {
    page_head(tr(POLL_RECORDED));
    echo tr(POLL_THANKS);
    page_tail();
} else {
    error();
}

?>
