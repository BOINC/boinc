<?php
    require_once("util.inc");
    require_once("login.inc");
    $head = sprintf("Login to %s", db_init());
    page_head("$head");
    print_login_form();
    page_tail();
?>
