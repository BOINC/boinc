<?php

    require_once("db.inc");
    require_once("util.inc");
    require_once("edit.inc");

    $authenticator = init_session();
    db_init();
    $user = get_user_from_auth($authenticator);
 
    if ($user) {
        page_head("Change Password"); 
	print_change_password($user);
        page_tail();
    } else {
        print_login_form();
    }

?> 
