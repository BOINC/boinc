<?php

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../project/project.inc");

function email_sent_message($email_addr) {
    if (defined('EMAIL_FROM')) {
        $email_from = EMAIL_FROM;
    } else {
        $email_from = URL_BASE;
    }

    page_head("Email sent");
    echo "
        Instructions have been emailed to $email_addr.
        <p>
        If the email doesn't arrive in a few minutes,
        your ISP may be blocking it as spam.
        In this case please contact your ISP and
        ask them to not block email from $email_from.
    ";
}

$email_addr = process_user_text(strtolower($_POST["email_addr"]));
if (!strlen($email_addr)) {
    error_page("no address given");
}
$user = lookup_user_email_addr($email_addr);

if (!$user) {
    page_head("No such user");
    echo "There is no user with email address $email_addr. <br>
        Try reentering your email address.<p>
    ";
} else {
	if (substr($user->authenticator, 0, 1) == 'x') {
		page_head("Account Currently Disabled");
		echo "This account has been administratively disabled.";
	} else {
		$user->email_addr = $email_addr;
		$retval = send_auth_email($user, false);
		if ($retval) {
			email_sent_message($email_addr);
		} else {
            page_head("Email failed");
			echo "Can't send email to $user->email_addr: $retval";
		}
	}
}

page_tail();

?>
