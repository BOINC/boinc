<?php
    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");
    require_once("login.inc");

    db_init();
    $user = get_user_From_cookie();
    if ($user) {
      	page_head("User stats");
    	show_user($user);
    } else {
	echo NOLOGIN;
    }
    page_tail();
?>
