<?php
    require_once("../inc/db.inc");
    require_once("../inc/util.inc");

    $next_url = $_GET["next_url"];
    
    db_init();
    $user = get_logged_in_user(false);

    page_head("Log in/out");
    print_login_form_aux($next_url, $user);

    page_tail();
?>
