<?php

include_once("util.inc");

function show_error($str) {
    page_head("Create account: error");
    echo $str;
    echo mysql_error();
    echo "<p>Click your browser's <b>Back</b> button to try again.";
    page_tail();
    exit();
}

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
        $email_addr,
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
    setcookie("auth", $authenticator);
    page_head("Account created");
    echo "Your account has been created,
        and an <b>account key</b> is being emailed to you.
        <p>
        If you don't already have it,
        <a href=download.php>download the BOINC client</a>.
        Install and run the client, and give it your account key.
        <p>
        If you're already running the BOINC client,
        select the <b>Add Project</b> command
        and give it your account key.";

    mail($new_email_addr, "Account key for ".PROJECT, "Your account key for ".PROJECT." is $authenticator.\nCopy this key into the BOINC client.");
    page_tail();
