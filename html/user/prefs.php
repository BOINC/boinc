<?php
    require_once("db.inc");
    require_once("util.inc");
    require_once("prefs.inc");

    $authenticator = init_session();
    db_init();

    $user = get_user_from_auth($authenticator);
    require_login($user);

    page_head("Preferences");
    print_prefs_display($user);
    page_tail();

?>
