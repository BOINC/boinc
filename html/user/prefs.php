<?php
    require_once("db.inc");
    require_once("util.inc");
    require_once("prefs.inc");

    db_init();

    $user = get_user_from_cookie();
    if ($user) {
        page_head("Preferences");
        print_prefs_display($user);
        page_tail();
    } else {
        print_login_form();
    }

?>
