<?php
    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");
    require_once("user.inc");

    $authenticator = init_session();
    db_init();
    $user = get_user_from_auth($authenticator);
    if ($user) {
        page_head("Hosts stats");
        show_hosts($user);
        page_tail();
    } else {
        print_login_form();
    }
?>
