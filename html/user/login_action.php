<?php
    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");

    db_init();
    $authenticator = $HTTP_POST_VARS["authenticator"];
    $email_addr = $HTTP_POST_VARS["email_addr"];
    $password = $HTTP_POST_VARS["password"];
    if (strlen($authenticator)) {
        $query = "select * from user where authenticator='$authenticator'";
    } else if (strlen($email_addr)) {
        $query = "select * from user where email_addr='$email_addr'";
    } else {
        echo "NO SELECTION";
    }
    $result = mysql_query($query);
    if ($result) {
        $user = mysql_fetch_object($result);
        mysql_free_result($result);
    }
    if (!$user) {
        page_head("Log in");
        echo "There is no account with the account key or email address you have entered.\n";
        echo "<b>Click <b>Back</b> to try again.\n"; 
    } else if (strlen($password)) {
        page_head("Log in");
        if ($user->web_password != $HTTP_POST_VARS["existing_password"]) {
            echo "Bad password.";
        }
    } else {
        setcookie("auth", $user->authenticator, time()+100000000);
        page_head("User Page");   
        show_user_page_private($user);
    }
    page_tail();
?>
