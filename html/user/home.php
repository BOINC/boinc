<?php
    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");

    // show the home page of whoever's logged in

    $authenticator = init_session();

    db_init();
    $user = get_user_from_auth($authenticator);
    if ($user) {
        page_head("User page for $user->name");
        show_user_page_private($user);
        page_tail();
    } else {
        print_login_form();
    }
?>
