<?php
    require_once("db.inc");
    require_once("util.inc");
    require_once("prefs.inc");

    $authenticator = init_session();
    db_init();

    $user = get_user_from_auth($authenticator);
    require_login($user);

    $subset = $_GET["subset"];
    page_head(subset_name($subset)." preferences");
    if ($subset == "global") {
        print_prefs_display_global($user);
    } else {
        print_prefs_display_project($user);
    }
    page_tail();

?>
