<?php
    require_once("../inc/util.inc");
    require_once("../inc/userw.inc");
    require_once("../inc/db.inc");
    require_once("../inc/wap.inc");
 
    $userid = get_int('id');
    db_init();

    $user = lookup_user_id($userid);
    if (!$user) {
        error_page("No such user");
    }
    show_user_wap($user);
?>
