<?php
    require_once("util.inc");
    require_once("user.inc");

    db_init();
    $user = get_user_from_cookie();
    if ($user) {
        show_user_page($user);
    } else {
        echo "Not logged in";
    }
?>
