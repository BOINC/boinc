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
        return $mail->Send();
    } else {
        if (defined('EMAIL_FROM')) {
            $headers = "From: ". EMAIL_FROM;
        } else {
            $headers ="";
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
        $subject = PROJECT." account confirmation for $user->name";
        $body = "Welcome to ".PROJECT.".

This email confirms your account with ".PROJECT.":
    Project URL:         $master_url
    User name:           $user->name
    E-mail:              $user->email_addr
    Account Key:         $user->authenticator
    (use this to log in if you forget your password)
";
    } else {
        $subject = PROJECT." information for $user->name";
        $body = "Your ".PROJECT." account information is:

    Project URL:         $master_url
    User name:           $user->name
    E-mail:              $user->email_addr
    Account Key:         $user->authenticator
    (use this to log in if you forget your password)
";
    }

$body .= "

For further information and assistance with ".PROJECT." go to
$master_url
";
    return send_email($user, $subject, $body);
}

// Function simply outputs some text letting the user know
// their authenticator was emailed to them.
//
function email_sent_message($email_addr) {
    if (defined('EMAIL_FROM')) {
        $email_from = EMAIL_FROM;
    } else {
        $email_from = URL_BASE;
    }

    echo "
        Your Account Key has been emailed to $email_addr.
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
