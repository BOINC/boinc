<?php
    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");
    require_once("login.inc");
    require_once("user.inc");

    db_init();
    $user = get_user_from_cookie();
    page_head("Hosts stats");
    if ($user) {
        show_hosts($user);
    } else {
        print_login_form();
    }
    page_tail();
?>
