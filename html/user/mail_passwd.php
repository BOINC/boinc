<?php

require_once("util.inc");
require_once("db.inc");

db_init();

page_head("Password");
if (strlen($HTTP_POST_VARS["Submit"])) {
    $query = sprintf(
        "select * from user where email_addr = '%s'",
        $HTTP_POST_VARS["email_addr"]
    );
    $result = mysql_query($query);
    $user = mysql_fetch_object($result);
    mysql_free_result($result);
    mail($user->email_addr, "BOINC password", "Your BOINC password is " . $user->web_password);
}

if (!$user) {
    echo "There is no user with that password. ";
    echo "Try reentering your email address.";
}

page_tail();

?>