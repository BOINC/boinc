<?php
    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");

    // show the home page of whoever's logged in

    db_init();
    $user = get_logged_in_user();
    page_head("User page", $user);
    show_user_page_private($user);
    page_tail();
?>
