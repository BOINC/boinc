<?php
    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");

    db_init();
    $user = get_user_from_cookie();
    if ($user) {
      	page_head("User stats");
    	show_user($user);
        page_tail();
    } else {
	print_login_form();
    }
?>
