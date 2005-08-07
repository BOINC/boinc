<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../inc/user.inc");

function send_verify_email($user, $email_addr, $key) {
    mail(
        $email_addr,
        PROJECT." account email change",
"You have asked that the email address of your " . PROJECT . " account be changed to $email_addr."
    );
}

db_init();
$user = get_logged_in_user();

$email_addr = strtolower(process_user_text(post_str("email_addr")));

page_head("Edit email address");
if (!is_valid_email_addr($email_addr)) {
    echo "Invalid email address requested";
} else if ($email_addr == $user->email_addr) {
    echo "No change requested";
} else {
    $existing = lookup_user_email_addr($email_addr);
    if ($existing) {
        echo "There's already an account with that email address";
    } else {
        $result = mysql_query("update user set email_addr='$email_addr', email_validated=0 where id=$user->id");
        if ($result) {
            echo "Email changed to $email_addr");
            send_verify_email($user, $email_addr, $x);
        } else {
            echo "
                We can't update your email address
                due to a database problem.  Please try again later.
            ";
        }
    }
}

page_tail();

?>
