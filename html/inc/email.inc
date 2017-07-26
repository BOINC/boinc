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
require_once("../project/project.inc");

// send an email, using PHPMailer or not.
//
function send_email($user, $subject, $body, $body_html=null) {
    if (function_exists("make_php_mailer")) {
        require_once("../inc/phpmailer/class.phpmailer.php");
        $mail = make_php_mailer();
        $mail->AddAddress($user->email_addr, $user->name);
        $mail->Subject = $subject;
        if ($body_html) {
            $mail->AltBody = $body;
            $mail->Body = $body_html;
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

// a valid email address is of the form A@B.C
// where A, B, C are nonempty,
// A and B don't contain @ or .,
// and C doesn't contain @ and is at least 2 chars
//
function is_valid_email_addr($addr) {
    if (defined("USE_STOPFORUMSPAM") && USE_STOPFORUMSPAM && array_key_exists('REMOTE_ADDR', $_SERVER)) {
        $ip = $_SERVER['REMOTE_ADDR'];
        // For obviously private IPs check just the email against SFS, otherwise check both IP and email
        if (filter_var($ip, FILTER_VALIDATE_IP, FILTER_FLAG_NO_PRIV_RANGE | FILTER_FLAG_NO_RES_RANGE)) {
            $x = @file_get_contents("https://www.stopforumspam.com/api?ip=".$ip."&email=".$addr);
        } else {
            $x = @file_get_contents("https://www.stopforumspam.com/api?email=".$addr);
        }
        if (substr_count($x, '<appears>yes</appears>')) {
            return false;
        }
    }
    $pattern = '/^([^@]+)@([^@\.]+)\.([^@]{2,})$/';
    $match = preg_match($pattern, $addr);
    return (bool) $match;
}

function salted_key($key) {
    return md5($key.'oogabooga');
}

function opt_out_url($user) {
    return secure_url_base()."opt_out.php?code=".salted_key($user->authenticator)."&userid=$user->id";
}
?>
