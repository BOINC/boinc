<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");

db_init();

page_head("Password");
$email_addr = trim(strtolower($HTTP_POST_VARS["email_addr"]));
if (strlen($email_addr)) {
    $esc_email_addr = escape_pattern("@".$email_addr."_");
     $query = "select * from user where email_addr = '$email_addr' "
        . "or email_addr like '$esc_email_addr'";
    $result = mysql_query($query);
    $user = mysql_fetch_object($result);
    mysql_free_result($result);
}

if (!$user) {
    echo "There is no user with that email address. <br>
        Try reentering your email address.<p>
    ";
} else {
    $retval = send_auth_email($user->email_addr, $user->authenticator);
    if ($retval) {
        email_sent_message($email_addr);
    } else {
        echo "Can't send email to $user->email_addr: $retval";
    }
}

page_tail();

?>
