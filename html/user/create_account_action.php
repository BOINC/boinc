<?php

include_once("util.inc");

function show_error($str) {
    page_head("Create account: error");
    echo $str;
    echo mysql_error();
    echo "<p>Click your browser's <b>Back</b> button to try again.\n<p>\n";
    page_tail();
    exit();
}

    $authenticator = init_session();
    db_init();

    $new_email_addr = $HTTP_POST_VARS["new_email_addr"];
    if (strlen($new_email_addr) == 0) {
        show_error("Email address missing");
    }
    $query = "select * from user where email_addr='$new_email_addr'";
    $result = mysql_query($query);
    if ($result) {
        $user = mysql_fetch_object($result);
        mysql_free_result($result);
        if ($user) {
            show_error("There's already an account with that email address");
        }
    }

    // web passwords disabled by default
    if (0) {
        if (strlen($HTTP_POST_VARS["new_password"]) == 0) {
            show_error("Password missing");
        }
        if ($HTTP_POST_VARS["new_password"] != $HTTP_POST_VARS["new_password2"]) {
            show_error("Different passwords entered");
        }
    }

    $authenticator = random_string();
    $query = sprintf(
       "insert into user (create_time, email_addr, name, web_password, authenticator, country, postal_code, total_credit, expavg_credit, expavg_time, teamid) values(%d, '%s', '%s', '%s', '%s', '%s', %d, 0, 0, 0, 0)",
        time(),
        $new_email_addr,
        $HTTP_POST_VARS["new_name"],
        $HTTP_POST_VARS["new_password"],
        $authenticator,
        $HTTP_POST_VARS["country"],
        $HTTP_POST_VARS["postal_code"]
    );
    $result = mysql_query($query);
    if (!$result) {
        show_error("Couldn't create account");
    }

    // In success case, redirect to a fixed page so that user can
    // return to it without getting "Repost form data" stuff

    send_auth_email($new_email_addr, $authenticator);
    Header("Location: account_created.php?email_addr=$new_email_addr");
