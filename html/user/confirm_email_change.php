<?php

    require_once("../inc/db.inc");
    require_once("../inc/util.inc");
    require_once("../inc/email.inc");

    db_init();

    $id = get_int("id");
    $str = process_user_text(get_str("str"));

    $user = null;
    $result = mysql_query("select * from user where id=$id");
    if ($result) {
        $user = mysql_fetch_object($result);
    }

    page_head("Verify email address change");
    if ($user) {
        if (split_munged_email_addr($user->email_addr, $str, $new_email)) {
            $new_email = trim(strtolower($new_email));
            $result = mysql_query("update user set email_addr='$new_email' where id=$user->id");
            if ($result) {
                echo "Email address change verified";
            } else {
                echo "Verification failed due to database error.  Please try again later.";
            }
        } else {
            $user = null;
        }
    } else {
        echo "User not found";
    }
    if (!$user) {
        echo "We weren't expecting a verification of this account's email address.  Please request the change again.";
    }
    page_tail();

?>
