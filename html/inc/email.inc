<?php

// email-related utilities.
// Don't put specific message text here.

require_once("../inc/util.inc");
require_once("../project/project.inc");

$config = get_config();
$master_url = parse_config($config, "<master_url>");

// Determine if phpmailer is installed and to be used (defined in project.inc)
if (isset($USE_PHPMAILER)) {
    if ($USE_PHPMAILER) {
        require_once("../inc/phpmailer/class.phpmailer.php");
    }
} else {
    $USE_PHPMAILER = false;
}

// Function sends an email, either using PHPMailer or not.
//
function send_email($user, $subject, $body, $body_html=null) {
    global $USE_PHPMAILER;
    global $PHPMAILER_HOST;
    global $PHPMAILER_MAILER;

    if ($USE_PHPMAILER) {
        $mail = new PHPMailer();
        $mail->AddAddress($user->email_addr, $user->name);
        $mail->Subject = $subject;
        if ($body_html) {
            $mail->AltBody = $body;
            $mail->Body = $body_html;
        } else {
            $mail->Body = $body;
        }
        $mail->From = EMAIL_FROM;
        $mail->FromName = EMAIL_FROM_NAME;
        $mail->Host = $PHPMAILER_HOST;
        $mail->Mailer = $PHPMAILER_MAILER;
        if (!$mail->Send()) {
            //echo $mail->ErrorInfo;
            return false;
        } else {
            return true;
        }
    } else {
        $headers ="";
        if (defined('EMAIL_FROM') && defined('EMAIL_FROM_NAME')) {
            $headers = "From: ".EMAIL_FROM_NAME." <".EMAIL_FROM.">";
        } else if (defined('EMAIL_FROM')) {
            $headers = "From: ". EMAIL_FROM;
        }
        return mail($user->email_addr, $subject, $body, $headers);
    }
}

// Send an email describing an account to the user.
// There are a few scenarios:
//
// 1) the account was created by user via web.
//    In this case they're currently looking at the "validate account" page
//    (account_created.php), although they might have strayed
//    so we need to give them a link.
// 2) the account was created administratively
// 3) the user requested account key for existing account
//
function send_auth_email($user, $is_new) {
    global $master_url;
    $body = "";

    if ($is_new) {
        $subject = PROJECT." account confirmation";
        $body = "Welcome to ".PROJECT.".

This email confirms your account with ".PROJECT.":
    Project URL:         $master_url
    User name:           $user->name
    E-mail:              $user->email_addr
";
    } else {
        $now = time();
        $x = md5($user->id.$user->authenticator.$now);
        $x = substr($x, 0, 16);
        $subject = PROJECT." account information";
        $body = "This email was sent in reponse to a request on the ".PROJECT." web site.

To log in to your ".PROJECT." account, visit:
".$master_url."login_action.php?id=$user->id&t=$now&h=$x
(This link is valid for 1 day).
After logging in, you can change your account's password or email address.
";
    }

$body .= "
For further information and assistance with ".PROJECT.", visit
$master_url
";
    return send_email($user, $subject, $body);
}

// a valid email address is of the form A@B.C
// where A, B, C are nonempty,
// A and B don't contain @ or .,
// and C doesn't contain @ and is at least 2 chars
//
function is_valid_email_addr($addr) {
    $pattern = '/^([^@]+)@([^@\.]+)\.([^@]{2,})$/';
    $match = preg_match($pattern, $addr);
    return (bool) $match;
}

function salted_key($key) {
    return md5($key.'oogabooga');
}

?>
