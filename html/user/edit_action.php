<?php

    $project = db_init();
    $head = sprintf("Updating %s Account", $project);
    $user = get_user_from_cookie();

    page_head($head);
    if ($user) {
        $my_name = $HTTP_POST_VARS["my_name"];
 	$my_country = $HTTP_POST_VARS["my_country"];
	$my_zip = $HTTP_POST_VARS["my_zip"];

        if (strlen($my_name) && strlen($my_country) && strlen($my_zip)) {
	    $query = sprintf("update user set name='%s', country='%s',  postal_code=%d where id=%d", $my_name, $my_country, $my_zip, $user->id);
	    $result = mysql_query($query);
	    if($result) {
		print_update_ok();
	    } else {
		print_update_fail();
	    }
        } else if (strlen($my_name) && strlen($my_country)) {
	    query = sprintf("update user set name='%s', country='%s' where id=%d", $my_name, $my_country, $user->id); 
            $result = mysql_query($query);
            if($result) {
                print_update_note();
            } else {
		print_update_fail();
	    }
        } else if (strlen($my_country) && strlen($my_zip)) { 
	   query = sprintf("update user set country='%s', postal_code='%s' where id=%d", $my_country, $my_zip, $user->id);
	   $result = mysql_query($query);
	   if($result) {
		print_update_note();
	   } else {
		print_update_fail(); 
	   }
        } else if (strlen($my_name) && strlen($my_zip)) {
	   query = sprintf("update user set name='%s', postal_code=%d where id=%d", $my_name, $my_zip, $user->id);  
           $result = mysql_query($query);
           if($result) {
                print_update_note();
           } else {
                print_update_fail();
           }
        } else if (strlen($my_name)) {
	   query = sprintf("update user set name='%s' where id=%d", $my_name, $user->id);  
           $result = mysql_query($query);
           if($result) {
                print_update_note();
           } else {
                print_update_fail();
           }
 
    	} else if (strlen($my_zip)) {
	   query = sprintf("update user set country='%s', postal_code='%s' where id=%d", $my_country, $my_zip, $user->id);  
           $result = mysql_query($query);
           if($result) {
                print_update_note();
           } else {
                print_update_fail();
           }

    	} else if (strlen($my_country)) {
	   query = sprintf("update user set country='%s', postal_code='%s' where id=%d", $my_country, $my_zip, $user->id);
           $result = mysql_query($query);
           if($result) {
                print_update_note();
           } else {
                print_update_fail();
           }
    	}



    $query = sprintf("select * from user where email_addr='%s',
		     $HTTP_POST_VARS["my_email"]);
    $result = mysql_query($query);
    if ($result) {
	$user = mysql_fetch_object($result);
   	mysql_free_result($result);
    }
    if ($user) {
  	printf(
	    TABLE2."\n"
	    ."<tr><td>There's already an account with that email address. Click the <b>Back</b> button\n"
	    ." on your browser to edit your information, or <a href=login.php>login </a>to your \n"
	    .$project." account.</td></tr>\n"
	    ."</table>\n"
	);
    } else {
	


