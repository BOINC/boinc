<?php

    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");
    require_once("edit.inc");

    $authenticator = init_session();
    db_init();
    $user = get_user_from_auth($authenticator);

    if (!$user) {
        print_login_form();
        exit();
    }
    page_head("Updating User Account");
    $my_email = $HTTP_POST_VARS["my_email"];
    $my_name = $HTTP_POST_VARS["my_name"];
    $my_country = $HTTP_POST_VARS["my_country"];
    $my_zip = $HTTP_POST_VARS["my_zip"];


    if (strlen($my_email)) {
        $query = sprintf("select * from user where email_addr='%s'", $my_email);
        $result = mysql_query($query);
        if ($result) {
            $old = mysql_fetch_object($result);
            mysql_free_result($result);
        }

        if ($old) {
            $email_ok = EMAIL_EXISTS;
        } else {
            srand((double)microtime*1000000);
            $new_pass = rand();
            $query = sprintf("update user set email_addr='%s', web_password ='%s' where id=%d", $my_email, $new_pass, $user->id);
            $result = mysql_query($query);
            if ($result) {
                $email_ok = EMAIL_UPDATED;
                mail($my_email, "NEW PASSWORD", "Your new temporary password is ".$new_pass.".\n\n"
                    ."You must use it as your password to access your account the next time you login."
                    ." Thereafter, you can change your password by clicking on the CHANGE PASSWORD link in your" 
                    ." Project User Page and use the changed password as your new permanent password.\n"
                );
            } else {
                $email_ok = EMAIL_FAIL;
            }
        }
    }

    if (strlen($my_name) && strlen($my_country) && strlen($my_zip)) {
        $query = sprintf("update user set name='%s', country='%s',  postal_code=%d where id=%d", $my_name, $my_country, $my_zip, $user->id);
        $result = mysql_query($query);
        if($result) {
            print_update_ok($email_ok);
        } else {
            print_update_fail($email_ok);
        }
    } else if (strlen($my_name) && strlen($my_country)) {
        $query = sprintf("update user set name='%s', country='%s' where id=%d", $my_name, $my_country, $user->id); 
        $result = mysql_query($query);
        if($result) {
            print_update_ok($email_ok);
        } else {
            print_update_fail($email_ok);
        }
    } else if (strlen($my_country) && strlen($my_zip)) { 
       $query = sprintf("update user set country='%s', postal_code='%s' where id=%d", $my_country, $my_zip, $user->id);
       $result = mysql_query($query);
       if($result) {
            print_update_ok($email_ok);
       } else {
            print_update_fail($email_ok); 
       }
    } else if (strlen($my_name) && strlen($my_zip)) {
       $query = sprintf("update user set name='%s', postal_code=%d where id=%d", $my_name, $my_zip, $user->id);  
       $result = mysql_query($query);
       if($result) {
            print_update_ok($email_ok);
       } else {
            print_update_fail($email_ok);
       }
    } else if (strlen($my_name)) {
       $query = sprintf("update user set name='%s' where id=%d", $my_name, $user->id);  
       $result = mysql_query($query);
       if($result) {
            print_update_ok($email_ok);
       } else {
            print_update_fail($email_ok);
       }

    } else if (strlen($my_country)) {
       $query = sprintf("update user set country='%s' where id=%d", $my_country, $user->id);  
       $result = mysql_query($query);
       if($result) {
            print_update_ok($email_ok);
       } else {
            print_update_fail($email_ok);
       }

    } else if (strlen($my_zip)) {
       $query = sprintf("update user set postal_code='%s' where id=%d", $my_zip, $user->id);
       $result = mysql_query($query);
       if($result) {
            print_update_ok($email_ok);
       } else {
            print_update_fail($email_ok);
       }
    }
    page_tail();

?>
