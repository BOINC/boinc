<?php
    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");
    require_once("login.inc");

    $project = db_init();
    $user = get_user_from_cookie();
    if ($user) {
        $head = sprintf("%s's User Page", $user->name);
        page_head($head);
        show_user_page($user, $project);
    } else {
        page_head("Log in");
        print_login_form();
    }
    page_tail();
?>
