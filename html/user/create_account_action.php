<?php

include_once("../inc/db.inc");
include_once("../inc/util.inc");

function show_error($str) {
    page_head("Create account: error");
    echo "$str<br>\n";
    echo mysql_error();
    echo "<p>Click your browser's <b>Back</b> button to try again.\n<p>\n";
    page_tail();
    exit();
}

    $config = get_config();
    if (parse_config($config, "<disable_account_creation/>")) {
        page_head("Account creation is disabled");
        echo "
            <h3>Account creation is disabled</h3>
            Sorry, this project has disabled the creation of new accounts.
            Please try again later.
        ";
        exit();
    }

    init_session();
    db_init();

    $userid = $_POST["userid"];
    if ($userid) {
        $result = mysql_query("select * from user where id=$userid");
        $clone_user = mysql_fetch_object($result);
        mysql_free_result($result);
        if (!$clone_user) {
            echo "User $userid not found";
            exit();
        }
        $teamid = $clone_user->teamid;
        $project_prefs = $clone_user->project_prefs;
    } else {
        $teamid = 0;
        $project_prefs = "";
    }

    $new_name = $_POST["new_name"];
    if (strlen($new_name)==0) {
        show_error("You must supply a name for your account");
    }

    $new_email_addr = trim($HTTP_POST_VARS["new_email_addr"]);
    $new_email_addr = strtolower($new_email_addr);
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
    $cross_project_id = random_string();
    $munged_email_addr = munge_email_addr($new_email_addr, $authenticator);
    $query = sprintf(
       "insert into user (create_time, email_addr, name, authenticator, country, postal_code, total_credit, expavg_credit, expavg_time, project_prefs, teamid, venue, url, send_email, show_hosts, cross_project_id) values(%d, '%s', '%s', '%s', '%s', '%s', 0, 0, 0, '$project_prefs', $teamid, 'home', '', 1, 1, '$cross_project_id')",
        time(),
        $munged_email_addr,
        $new_name,
        $authenticator,
        $_POST["country"],
        $_POST["postal_code"]
    );
    $result = mysql_query($query);
    if (!$result) {
        show_error("Couldn't create account");
    }

    // In success case, redirect to a fixed page so that user can
    // return to it without getting "Repost form data" stuff

    send_auth_email($new_email_addr, $authenticator);
    Header("Location: account_created.php?email_addr=$new_email_addr");
