<?php

    require_once("edit.inc");
    require_once("util.inc");

    $project = db_init();
    $user = get_user_from_cookie();

    if ($user) {
    	$head = sprintf("Edit %s's User Information", $user->name);
    	page_head($head);
    	print_edit_user_info($user);
    } else {
    	page_head("Edit User Information"); 
    	printf("Not Logged in. Click <a href=login.php>here</a> to login.\n");
    }
    page_tail();

?>
    



