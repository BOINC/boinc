<?php

    require_once("edit.inc");
    require_once("util.inc");

    db_init();
    $authenticator = init_session();
    $user = get_user_from_auth($authenticator);

    if ($user) {
    	$head = sprintf("Edit %s's User Information", $user->name);
    	page_head($head);
    	print_edit_user_info($user);
        page_tail();
    } else {
	print_login_form();
    }

?>
    



