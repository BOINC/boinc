<?php

    require_once("db.inc");
    require_once("util.inc");
    require_once("edit.inc");

    db_init();
    $user = get_user_from_cookie();
 
    if ($user) {
        page_head("Change Password"); 
	print_change_password($user);
        page_tail();
    } else {
        print_login_form();
    }

?> 
