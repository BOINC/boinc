<?php

include_once("util.inc");

function show_error($str) {
    page_head("Create account: error");
    echo "$str<br>\n";
    echo mysql_error();
    echo "<p>Click your browser's <b>Back</b> button to try again.\n<p>\n";
    page_tail();
    exit();
}

    $authenticator = init_session();
    db_init();

    $new_name = $HTTP_POST_VARS["new_name"];
    if (strlen($new_name)==0) {
        show_error("You must supply a name for your account");
    }

    $new_email_addr = $HTTP_POST_VARS["new_email_addr"];
    if (!is_valid_email_addr($new_email_addr)) {
        show_error("Invalid email address:
            you must enter a valid address of the form
            name@domain"
        );
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

    $authenticator = random_string();
    $munged_email_addr = munge_email_addr($new_email_addr, $authenticator);
    $query = sprintf(
       "insert into user (create_time, email_addr, name, web_password, authenticator, country, postal_code, total_credit, expavg_credit, expavg_time, teamid, venue) values(%d, '%s', '%s', '%s', '%s', '%s', '%s', 0, 0, 0, 0, 'home')",
        time(),
        $munged_email_addr,
        $new_name,
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
