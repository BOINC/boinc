<?php
    require_once("db.inc");
    require_once("util.inc");
    require_once("login.inc");
    require_once("prefs.inc");

    db_init();

    $user = get_user_from_cookie();
    if ($user) {
        page_head("Preferences");
        $prefs = prefs_parse($user->prefs);
        print_prefs_display($prefs);
    } else {
        print_login_form();
    }
    echo "<p>\n";
    page_tail();

?>
