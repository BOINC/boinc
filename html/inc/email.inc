<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// email-related utilities.
// Don't put specific message text here.

require_once("../inc/util.inc");
require_once("../inc/token.inc");
require_once("../project/project.inc");
require_once("../inc/token.inc");

// send an email, using PHPMailer or not.
//
function send_email($user, $subject, $body, $body_html=null, $email_addr=null) {
    if (!$email_addr) {
        $email_addr = $user->email_addr;
    }
    if (defined('EMAIL_USE_CMD')) {
        if ($body_html) {
            $cmd = "mail  -a \"Content-type: text/html\" -s \"$subject\" $email_addr";
            $pipe = popen($cmd, "w");
            fwrite($pipe, $body_html);
        } else {
            $cmd = "mail  -s \"$subject\" $email_addr";
            $pipe = popen($cmd, "w");
            fwrite($pipe, $body);
        }
        pclose($pipe);
        return true;
    } else if (function_exists("make_php_mailer")) {
        if (file_exists("../inc/PHPMailer/src/PHPMailer.php") && file_exists("../inc/PHPMailer/src/SMTP.php")) {
            require_once("../inc/PHPMailer/src/PHPMailer.php");
            require_once("../inc/PHPMailer/src/SMTP.php");
        } else if (file_exists("../inc/phpmailer/class.phpmailer.php")) {
            require_once("../inc/phpmailer/class.phpmailer.php");
        } else {
            echo "PHPMailer not installed";
            return false;
        }
        $mail = make_php_mailer();
        $mail->AddAddress($email_addr, $user->name);
        $mail->Subject = $subject;
        if ($body_html) {
            $mail->AltBody = $body;
            $mail->Body = $body_html;
            $mail->IsHTML(true);
        } else {
            $mail->Body = $body;
        }
        if (!$mail->Send()) {
            echo $mail->ErrorInfo;
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
        if ($body_html) {
            $body = "<html><body>\n";
            $body .= $body_html;
            $body .= "\n</body></html>\n";
            $headers .= 'Content-type: text/html; charset=UTF-8' . "\r\n";
        }
        return mail($email_addr, $subject, $body, $headers);
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
function send_auth_email($user) {
    $body = "";

    $now = time();
    $x = md5($user->id.$user->authenticator.$now);
    $x = substr($x, 0, 16);
    $subject = PROJECT." account information";
    $body = "This email was sent in response to a request on the ".PROJECT." web site.

To log in to your ".PROJECT." account, visit:
".secure_url_base()."login_action.php?id=$user->id&t=$now&h=$x
(This link is valid for 1 day).
After logging in, you can change your account's password or email address.
";

$body .= "
For further information and assistance with ".PROJECT.", visit
".secure_url_base()."
";

    return send_email($user, $subject, $body);
}

function send_changed_email($user) {
    $duration = TOKEN_DURATION_ONE_WEEK;

    $token = create_token($user->id, TOKEN_TYPE_CHANGE_EMAIL, $duration);

    $subject = PROJECT." email address change.";

    // Body for the new email address to explain how quickly
    // they can do another email change.
    //
    $body_new = "Your email address was changed from ".$user->previous_email_addr.
" to ".$user->email_addr." on ".date('F j \a\t g:i a T', $user->email_addr_change_time).
".  You will not be able to change your email address again until ".date('F j \a\t g:i a T', $user->email_addr_change_time + $duration).
".  If you need to undo this immediately, please look for an email from us at your ".$user->previous_email_addr." address.";

    // We need to send a different version of the email to the old address.
    //
    $body_old = "Your email address has been changed. If you did not take this action,
then please click on the link below to reverse this process and change your password.

".secure_url_base()."recover_email.php?id=".$user->id."&token=".$token."

Note:  Your password will need to be recovered after clicking this link";

    return send_email($user, $subject, $body_new) && send_email($user, $subject, $body_old, null, $user->previous_email_addr);
}

// return true if the email address
// - is syntactically valid according to filter_var()
// - if configured, is OK with stopforumspam.com
//
function is_valid_email_sfs($addr) {
    if (!defined("USE_STOPFORUMSPAM")) return true;
    if (!USE_STOPFORUMSPAM) return true;
    $ip = '';
    if (array_key_exists('REMOTE_ADDR', $_SERVER)) {
        $ip = $_SERVER['REMOTE_ADDR'];
    }
    // For obviously private IPs check just the email against SFS,
    // otherwise check both IP and email
    if ($ip && filter_var(
        $ip, FILTER_VALIDATE_IP,
        FILTER_FLAG_NO_PRIV_RANGE | FILTER_FLAG_NO_RES_RANGE
    )) {
        $x = @file_get_contents("https://www.stopforumspam.com/api?ip=".$ip."&email=".$addr);
    } else {
        $x = @file_get_contents("https://www.stopforumspam.com/api?email=".$addr);
    }
    // could also look at 'frequency' and 'lastseen'
    // see https://www.stopforumspam.com/usage
    //
    if (substr_count($x, '<appears>yes</appears>')) {
        error_log("stopforumspam.com rejected email $addr, IP $ip");
        return false;
    }
    return true;
}

function is_valid_email_syntax($addr) {
    return filter_var($addr, FILTER_VALIDATE_EMAIL);
}

function send_confirm_delete_email($user) {
    $token = create_token($user->id, TOKEN_TYPE_DELETE_ACCOUNT, TOKEN_DURATION_ONE_DAY);
    if ($token === null) {
        error_page("Error creating token.  Please try again later.");
    }

    $subject = "Confirm your request to delete your account at ".PROJECT;
    $body = "This email was sent in response to a request on the ".PROJECT." web site.

You have requested to delete your account at ".PROJECT.". In order to do this, use the following link to confirm your intent to delete your account. ".
"The link will take you to a web page where you will be asked to enter your password and complete the process of deleting your account.

".secure_url_base()."delete_account_confirm.php?id=$user->id&token=$token

This link is valid for 1 day.

For further information and assistance with ".PROJECT.", visit ".secure_url_base();

    return send_email($user, $subject, $body);
}

function salted_key($key) {
    return md5($key.'oogabooga');
}

function opt_out_url($user, $page="opt_out.php") {
    return sprintf("%s%s?code=%s&userid=%d",
        secure_url_base(),
        $page,
        salted_key($user->authenticator),
        $user->id
    );
}
?>
