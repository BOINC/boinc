<?php

require_once("../inc/util.inc");
require_once("../project/project.inc");

// Sends the authenticator to the given email address
//
function send_auth_email($email_addr, $auth) {
    $user = get_user_from_auth($auth);
    $subject = PROJECT." new account confirmation for $user->name";
    if (defined('EMAIL_FROM')) {
        $headers = "From: ". EMAIL_FROM;
    } else {
        $headers ="";
    }
    $body = "Welcome to ".PROJECT.".
        
This email confirms your account with ".PROJECT.":

    Project URL:         ".MASTER_URL."
    Account ID:          $auth

    User name:           $user->name
    E-mail:              $email_addr

You will need the Project URL and Account ID to attach the 
BOINC client to the project.    

You will need  your Account ID to login to the ".PROJECT." web site,
or you can use this direct link:

".URL_BASE."login_action.php?key=$auth

Please save this email.    For further information and assistance
with ".PROJECT." go to ".MASTER_URL."
";
    return mail($email_addr, $subject, $body, $headers);
}

function email_sent_message($email_addr) {
    if (defined('EMAIL_FROM')) {
        $email_from = EMAIL_FROM;
    } else {
        $email_from = URL_BASE;
    }

    echo "
        Your account ID has been emailed to $email_addr.
        <p>
        If the email doesn't arrive in a few minutes,
        your ISP may be blocking it as spam.
        In this case please contact your ISP and
        ask them to not block email from $email_from.
    ";
}

// a valid email address is of the form A@B.C
// where A, B, C are nonempty,
// A and B don't contain @ or .,
// and C doesn't contain @
//
function is_valid_email_addr($addr) {
    $x = strstr($addr, "@");
    if (!$x) return false;
    if (strlen($x) == strlen($addr)) return false;
    $x = substr($x, 1);
    if (strstr($x, "@")) return false;
    $y = strstr($x, ".");
    if (!$y) return false;
    if (strlen($y) == strlen($x)) return false;
    if (strlen($y) == 1) return false;
    return true;
}

// A few functions relating to email-address munging
// A "munged" email address is of the form @X_Y,
// where X is a valid email address
// and Y is a random string not containing _.
// When an email address hasn't been validated yet, it's munged.
// (Used during account creation and email address changes)

function munge_email_addr($email, $string) {
    return "@".$email."_".$string;
}

// if email_addr is of the form @X_Y, split out the X and return true.
// otherwise return false
//
function split_munged_email_addr($addr, $auth, &$email) {
    if (substr($addr, 0, 1) != "@") return false;
    $x = strrchr($addr, "_");
    if (!$x) return false;
    $y = substr($x, 1);
    if ($auth && $y != $auth) return false;
    $email = substr($addr, 1, strlen($addr)-strlen($x)-1);
    return true;
}

?>
