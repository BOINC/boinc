<?php
    require_once("../inc/util.inc");
    require_once("../inc/user.inc");
    require_once("../inc/db.inc");

    // show the home page of whoever's logged in

    db_init();
    $user = get_logged_in_user();
    page_head("Your account");
    show_user_page_private($user);
    page_tail();
?>
