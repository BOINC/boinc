<?php

include_once("db.inc");
include_once("util.inc");

session_start();
session_destroy();

if ($_COOKIE['auth']) {
    setcookie('auth', "", time());
}

page_head("Logged out");

echo "You are now logged out";

page_tail();
?>
