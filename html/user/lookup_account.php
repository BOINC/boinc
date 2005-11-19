<?php

// RPC handler for account lookup

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../inc/xml.inc");

db_init();

xml_header();

$email_addr = get_str("email_addr");
$passwd_hash = process_user_text(get_str("passwd_hash"));

$user = lookup_user_email_addr($email_addr);

// if no password set, make it the account key
//
if ($user && !strlen($user->passwd_hash)) {
    $user->passwd_hash = md5($user->authenticator.$user->email_addr);
    mysql_query("update user set passwd_hash='$user->passwd_hash' where id=$user->id");
}

if (!$user || $user->passwd_hash != $passwd_hash) {
    echo "<account_out>\n";
    echo "<error_num>-161</error_num>\n";
    echo "</account_out>\n";
} else {
    echo "<account_out>\n";
    echo "<authenticator>$user->authenticator</authenticator>\n";
    echo "</account_out>\n";
}

?>

