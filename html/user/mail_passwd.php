<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");

db_init();

page_head("Password");
$email_addr = trim(strtolower($HTTP_POST_VARS["email_addr"]));
if (strlen($email_addr)) {
    $query = sprintf(
        "select * from user where email_addr = '%s'",
        $email_addr
    );
    $result = mysql_query($query);
    $user = mysql_fetch_object($result);
    mysql_free_result($result);
    send_auth_email($user->email_addr, $user->authenticator);
}

if (!$user) {
    echo "There is no user with that email address. <br>
        Try reentering your email address.<p>
    ";
} else {
    echo "Your account ID has been emailed to ".$email_addr.".<p>";
}

page_tail();

?>
