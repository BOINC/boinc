<?php

include_once("../inc/db.inc");
include_once("../inc/util.inc");

session_start();
session_destroy();

if (isset($_COOKIE['auth'])) {
    setcookie('auth', "", time());
}

page_head("Logged out");

echo "You are now logged out";

page_tail();
?>
