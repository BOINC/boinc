<?php

    require_once("../inc/db.inc");
    require_once("../inc/util.inc");
    require_once("../inc/email.inc");

    db_init();

    $id = get_int("id");
    $str = process_user_text(get_str("str"));

    $user = lookup_user_id($id);
    if (!$user) {
        error_page("No such user");
    }

    page_head("Verify email address change");
    if (split_munged_email_addr($user->email_addr, $str, $new_email)) {
        $new_email = trim(strtolower($new_email));
        $result = mysql_query("update user set email_addr='$new_email' where id=$user->id");
        if ($result) {
            echo "Email address change verified";
        } else {
            echo "Verification failed due to database error.  Please try again later.";
        }
    } else {
        echo "
            We weren't expecting a verification of this account's email address.
            Please request the change again.
        ";
    }
    page_tail();

?>
