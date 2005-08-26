<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");

db_init();

$email_addr = process_user_text(strtolower($_POST["email_addr"]));
if (!strlen($email_addr)) {
    error_page("no address given");
}
$user = lookup_user_email_addr($email_addr);

if (!$user) {
    page_head("No such user");
    echo "There is no user with email address $email_addr. <br>
        Try reentering your email address.<p>
    ";
} else {
    $user->email_addr = $email_addr;
    page_head("Mailed account key");
    $retval = send_auth_email($user, false);
    if ($retval) {
        email_sent_message($email_addr);
    } else {
        echo "Can't send email to $user->email_addr: $retval";
    }
}

page_tail();

?>
