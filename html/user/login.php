<?php
    require_once("util.inc");
    require_once("login.inc");
    db_init();
    page_head("Log in");
    print_login_form();
?>
