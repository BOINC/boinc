<?php
    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");
    require_once("login.inc");

    $project = db_init();
    $user = get_user_from_cookie();
    if ($user) {
        $head = sprintf("%s's User Page for %s", $user->name, $project);
        page_head($head);
        show_user_page($user, $project);
    } else {
        $head = sprintf("Login to %s", $project);
        page_head($head);
        print_login_form();
    }
    page_tail();
?>
