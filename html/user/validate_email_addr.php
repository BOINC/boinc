<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");

db_init();

function send_validate_email() {
    global $master_url;
    $user = get_logged_in_user();
    $x2 = md5($user->email_addr.$user->authenticator);
    send_email(
        $user,
        "Validate BOINC email address",
        "Please visit the following link to validate the email address\n"
        ."of your ".PROJECT." account:\n"
        .$master_url."validate_email_addr.php?validate=1&u=$user->id&x=$x2"
    );
    page_head("Validate email sent");
    echo "
        An email has been sent to $user->email_addr.
        Visit the link it contains to validate your email address.
    ";
    page_tail();
}

function validate() {
    $x = process_user_text(get_str("x"));
    $u = process_user_text(get_int("u"));
    $user = lookup_user_id($u);
    if (!$user) {
        error_page("No such user.\n");
    }

    $x2 = md5($user->email_addr.$user->authenticator);
    if ($x2 != $x) {
        error_page("Error in URL data - can't validate email address");
    }

    $result = mysql_query("update user set email_validated=1 where id=$user->id");
    if (!$result) {
        error_page("Database update failed - please try again later.");
    }

    page_head("Validate email address");
    echo "
        The email address of your account has been validated.
    ";
    page_tail();
}

if ($_GET['validate']) {
    validate();
} else {
    send_validate_email();
}

?>
