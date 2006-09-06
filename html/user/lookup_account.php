<?php

// RPC handler for account lookup

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../inc/xml.inc");

xml_header();
$retval = db_init_xml();
if ($retval) xml_error($retval);

$email_addr = get_str("email_addr");
$passwd_hash = process_user_text(get_str("passwd_hash", true));

$user = lookup_user_email_addr($email_addr);
if (!$user) {
    xml_error(-136);
}

if (!$passwd_hash) {
    echo "<account_out>
    <success/>
</account_out>
";
    exit();
}

$auth_hash = md5($user->authenticator.$user->email_addr);

// if no password set, set password to account key
//
if (!strlen($user->passwd_hash)) {
    $user->passwd_hash = $auth_hash;
    mysql_query("update user set passwd_hash='$user->passwd_hash' where id=$user->id");
}

// if the given password hash matches (auth+email), accept it
//
if ($user->passwd_hash == $passwd_hash || $auth_hash == $passwd_hash) {
    echo "<account_out>\n";
    echo "<authenticator>$user->authenticator</authenticator>\n";
    echo "</account_out>\n";
} else {
    xml_error(-206);
}

?>

