<?php

require_once("../inc/util.inc");
require_once("../project/project.inc");

// Send an email describing an account to the user.
// There are a few scenarios:
//
// 1) the account was created by user via web.
//    In this case they're currently looking at the "validate account" page
//    (account_created.php), although they might have strayed
//    so we need to give them a link.
// 2) the account was created administratively and is not validated.
// 3) the account was created administratively and is validated.
// 4) the user requested account key for existing account
//
function send_auth_email($user, $is_new, $is_validated) {
    $body = "";

    if (defined('EMAIL_FROM')) {
        $headers = "From: ". EMAIL_FROM;
    } else {
        $headers ="";
    }

    if ($is_new) {
        $subject = PROJECT." account confirmation for $user->name";
        $body = "Welcome to ".PROJECT.".

This email confirms your account with ".PROJECT.":
    Project URL:         ".MASTER_URL."
    Account Key:         $user->authenticator

    User name:           $user->name
    E-mail:              $user->email_addr
";
        if (!$is_validated) {
            $body .= "
To configure your account, visit ".URL_BASE."account_created.php
";
        }
    } else {
        $subject = PROJECT." information for $user->name";
        $body = "Your ".PROJECT." account information is:
    Project URL:         ".MASTER_URL."
    Account Key:         $user->authenticator

    User name:           $user->name
    E-mail:              $user->email_addr
";
    }

$body .= "

Save this email.
You'll need the Project URL and Account Key when you run the client software
and to log in to the ".PROJECT." web site.

For further information and assistance with ".PROJECT." go to ".MASTER_URL."
";
    return mail($user->email_addr, $subject, $body, $headers);
}

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

// Given email address X,
// find a user record whose email_addr field is a munged version of X,
// i.e. of the form @X_Y (Y is an authenticator, and contains no _s)
// There may be more than one.  Return any.
//
function lookup_user_munged_email($email) {
    $pattern = escape_pattern("@".$email."_");
    $q = "select * from user where email_addr like '$esc_email_addr%'";
    $result = mysql_query($q);
    $found = false;
    while ($user = mysql_fetch_object($result)) {

        // make sure we don't return @X_a_Y
        //
        $x = strrchr($user->email_addr, "_");
        $y = "@".$email.$x;
        if ($y == $user->email_addr) {
            $found = true;
            break;
        }
    }
    mysql_free_result($result);
    if (!$found) {
        return null;
    }
    return $user;
}

function validated($user) {
    if (substr($user->email_addr, 0, 1) == "@") return false;
    return true;
}

//////////////////////////////////////////////

function send_moderation_email($user, $thread,  $post, $message) {
    $body = "";

    if (defined('EMAIL_FROM')) {
        $headers = "From: ". EMAIL_FROM;
    } else {
        $headers ="";
    }
    $subject = PROJECT." forum moderation notice for $user->name";
    $body = PROJECT." notification:

This email is sent to inform you that one of your posts in the forum has been affected by moderation in ".PROJECT.":
    Thread:         ".$thread->title."
    Link:           ".MASTER_URL."/forum_thread.php?id=".$thread->id."
    
The moderator gave this explanation to why your post was moderated:
".$message;
$body .= "

This was the contents of your post:
".$post->content."

For further information and assistance with ".PROJECT." go to ".MASTER_URL;
    return mail($user->email_addr, $subject, $body, $headers);

}

function send_thread_moderation_email($user, $thread, $message) {
    $body = "";

    if (defined('EMAIL_FROM')) {
        $headers = "From: ". EMAIL_FROM;
    } else {
        $headers ="";
    }
    $subject = PROJECT." forum moderation notice for $user->name";
    $body = PROJECT." notification:

This email is sent to inform you that one of your threads in the forum has been affected by moderation in ".PROJECT.":
    Thread:         ".$thread->title."
    Link:           ".MASTER_URL."/forum_thread.php?id=".$thread->id."
    
The moderator gave this explanation to why your thread was moderated:
".$message;
$body .= "

For further information and assistance with ".PROJECT." go to ".MASTER_URL;
    return mail($user->email_addr, $subject, $body, $headers);

}


?>
