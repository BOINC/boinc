<?php

    require_once("edit.inc");
    require_once("util.inc");

    $project = db_init();
    $user = get_user_from_cookie();

    if ($user) {
    	$head = sprintf("Edit %s's %s User Information", $user, $project);
    	page_head($head);
    	print_edit_user_info($user);
    } else {
    	$head = sprintf("Edit %s User Information", $project);
    	page_head($head); 
    	printf("Not Logged in. Click <a href=login.php>here</a> to login.\n");
    }
    page_tail();

?>
    



