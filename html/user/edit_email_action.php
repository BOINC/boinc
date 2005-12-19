<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../inc/user.inc");

db_init();
$user = get_logged_in_user();

$email_addr = strtolower(process_user_text(post_str("email_addr")));
$passwd = process_user_text(post_str("passwd", true));

page_head("Change email address of account");

if (!is_valid_email_addr($email_addr)) {
    echo "New email address '$email_addr' is invalid";
} else if ($email_addr == $user->email_addr) {
    echo "New email address is same as existing address; no change.";
} else {
    $existing = lookup_user_email_addr($email_addr);
    if ($existing) {
        echo "There's already an account with that email address";
    } else {
        $passwd_hash = md5($passwd.$user->email_addr);

        // deal with the case where user hasn't set passwd
        // (i.e. passwd is account key)
        //
        if ($passwd_hash != $user->passwd_hash) {
            $passwd = $user->authenticator;
            $passwd_hash = md5($passwd.$user->email_addr);
        }
        if ($passwd_hash != $user->passwd_hash) {
            echo "Invalid password.";
        } else {
            $passwd_hash = md5($passwd.$email_addr);
            $query = "update user set email_addr='$email_addr', passwd_hash='$passwd_hash', email_validated=0 where id=$user->id";
            $result = mysql_query($query);
            if ($result) {
                echo "
                    The email address of your account is now
                    $email_addr.
                    <p>
                    We have send an email message to that address.
                    <p>
                    To validate the new address, visit the link in the email.
                ";
                send_verify_email($user->email_addr, $email_addr, $user);
            } else {
                echo "
                    We can't update your email address
                    due to a database problem.  Please try again later.
                ";
            }
        }
    }
}

page_tail();

?>
