<?php
    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");

    db_init();
    if (strlen($HTTP_POST_VARS["old"])) {
        $query = sprintf(
            "select * from user where email_addr='%s'",
            $HTTP_POST_VARS["existing_email"]
        );
        $result = mysql_query($query);
        if ($result) {
            $user = mysql_fetch_object($result);
            mysql_free_result($result);
        }
        if (!$user or ($user->web_password != $HTTP_POST_VARS["existing_password"])) {
	    page_head("Logging In");
            echo "We have no account with that name and password.";
        } else {
            setcookie("auth", $user->authenticator, time()+100000000);
            page_head("User home");
            show_user_page($user);
        }
    } else if (strlen($HTTP_POST_VARS["new"])) {
        $query = sprintf(
            "select * from user where email_addr='%s'",
            $HTTP_POST_VARS["new_email_addr"]
        );
        $result = mysql_query($query);
        if ($result) {
            $user = mysql_fetch_object($result);
            mysql_free_result($result);
        }
        if ($user) {
	    page_head("Creating Account");
            echo "There's already an account with that email address.";
        } else {
            if ($HTTP_POST_VARS["new_password"] != $HTTP_POST_VARS["new_password2"]) {
		page_head("Creating Account");
                echo "You've typed two different passwords.";
            } else {
                $authenticator = random_string();
                $email_addr = $HTTP_POST_VARS["new_email_addr"];
                $query = sprintf(
                   "insert into user (create_time, email_addr, name, web_password, authenticator, country, postal_code) values(%d, '%s', '%s', '%s', '%s', '%s', %d)",
                    time(),
                    $email_addr,
                    $HTTP_POST_VARS["new_name"],
                    $HTTP_POST_VARS["new_password"],
                    $authenticator,
                    $HTTP_POST_VARS["country"],
                    $HTTP_POST_VARS["postal_code"]
                );
                $result = mysql_query($query);
                if ($result) {
                    setcookie("auth", $authenticator);
		    page_head("Creating Account");
                    echo "Account created.  You are being mailed a key that you'll need to run the client.\n";
                    mail($email_addr, "BOINC key", "Your BOINC key is " . $authenticator);
                } else {
	       	    page_head("Creating Account");
                    echo "Couldn't create account - please try later.\n";
                }
            }
        }
    }
    page_tail();
?>
