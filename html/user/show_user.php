<?php
    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");
    require_once("login.inc");

    db_init();
    $user = get_user_from_cookie();
    page_head("User stats");
    if ($user) {
        show_user($user);
    } else {
        print_login_form();
    }
    page_tail();
?>
