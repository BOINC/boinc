<?php
    require_once("util.inc");
    require_once("login.inc");
    db_init();
    if ($user = get_user_from_cookie()) {
    	show_login($user);
    } else {
    	print_login_form();
    }
?>
