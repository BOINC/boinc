<?php
    require_once("util.inc");
    require_once("login.inc");
    db_init();
    page_head("Log in");
    $user = get_user_from_cookie();
    show_login($user);
    print_login_form();
?>
