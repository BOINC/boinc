<?php

include_once("util.inc");

session_start();
session_destroy();

page_head("Logged out");

echo "You are now logged out";

page_tail();
?>
