<?php

    require_once("edit.inc");
    require_once("util.inc");

    db_init();
    $user = get_user_from_cookie();

    if ($user) {
    	$head = sprintf("Edit %s's User Information", $user->name);
    	page_head($head);
    	print_edit_user_info($user);
        page_tail();
    } else {
	print_login_form();
    }

?>
    



