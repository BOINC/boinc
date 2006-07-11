<?php
require_once("poll.inc");

mysql_pconnect("localhost", "boincadm", null);
mysql_select_db("poll");

session_start();
$uid = session_id();

function parse_form() {
}

function generate_xml($x) {
}

function parse_xml($xml) {
}


?>
