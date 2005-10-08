<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/user.inc");

db_init();

function is_ascii($str) {
    // the mb_* functions are not included by default
    // return (mb_detect_encoding($passwd) -= 'ASCII');
    
    for ($i=0; $i<strlen($str); $i++) {
        $c = ord(substr($str, $i));
        if ($c < 32 || $c > 127) return false;
    }
    return true;
}

$auth = process_user_text(post_str("auth", true));
$email_addr = strtolower(process_user_text(post_str("email_addr", true)));

// Note: don't call process_user_text() on passwords.
// This is not needed, and will break passwords containing punctuation

$old_passwd = stripslashes(post_str("old_passwd", true));
$passwd = stripslashes(post_str("passwd"));
$passwd2 = stripslashes(post_str("passwd2"));

if ($passwd != $passwd2) {
    error_page("New passwords are different");
}

$config = get_config();
$min_passwd_length = parse_config($config, "<min_passwd_length>");
if (!$min_passwd_length) $min_passwd_length = 6;

if (!is_ascii($passwd)) {
    error_page("Passwords may only include ASCII characters.");
}

if (strlen($passwd)<$min_passwd_length) {
    error_page(
        "New password is too short:
        minimum password length is $min_passwd_length characters."
    );
}
if ($auth) {
    $user = lookup_user_auth($auth);
    if (!$user) {
        error_page("Invalid account key");
    }
} else {
    $user = lookup_user_email_addr($email_addr);
    if (!$user) {
        error_page("No account with that email address was found");
    }
    $passwd_hash = md5($old_passwd.$email_addr);
    if ($user->passwd_hash != $passwd_hash) {
        error_page("Invalid password");
    }
}

page_head("Change password");
$passwd_hash = md5($passwd.$user->email_addr);
$query = "update user set passwd_hash='$passwd_hash' where id=$user->id";
$result = mysql_query($query);
if ($result) {
    echo "Your password has been changed.";
} else {
    echo "
        We can't update your password due to a database problem.
        Please try again later.
    ";
}

page_tail();
?>
