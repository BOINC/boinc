<?php

    require_once("edit.inc");
    require_once("util.inc");

    db_init();
    $authenticator = init_session();
    $user = get_user_from_auth($authenticator);
    require_login($user);

    	$head = sprintf("Edit %s's User Information", $user->name);
    	page_head($head);
    	print_edit_user_info($user);
        page_tail();

?>
    



