<?php
    require_once("util.inc");
    require_once("login.inc");

    db_init();
    $head = sprintf("Login to %s", PROJECT);
    page_head("$head");
    print_login_form();
    page_tail();
?>
