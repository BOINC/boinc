<?php

    require_once("db.inc");
    require_once("util.inc");
    require_once("edit.inc");

    db_init();
    $user = get_user_From_cookie();
 
    page_head("Change Password"); 
    if ($user) {
	print_change_password($user);
    } else {
        echo NOLOGIN;
    }
    page_tail();

?> 
