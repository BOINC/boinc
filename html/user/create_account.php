<?php

include_once("db.inc");
include_once("util.inc");
include_once("login.inc");
include_once("prefs.inc");

db_init();

page_head("Create Account");
print_create_account_form();
page_tail();

?>
