<?php

// RPC handler for account creation

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../inc/xml.inc");

xml_header();

$retval = db_init_xml();
if ($retval) xml_error($retval);

$config = get_config();
if (parse_bool($config, "disable_account_creation")) {
    xml_error(-208);
}

if(defined('INVITE_CODES')) {
    $invite_code = process_user_text(get_str("invite_code"));
    if (!preg_match(INVITE_CODES, $invite_code)) {
	xml_error(-209);
    }
} 

$email_addr = get_str("email_addr");
$email_addr = process_user_text(strtolower($email_addr));
$passwd_hash = process_user_text(get_str("passwd_hash"));
$user_name = process_user_text(get_str("user_name"));

if (!is_valid_email_addr($email_addr)) {
    xml_error(-205);
}

if (strlen($passwd_hash) != 32) {
    xml_error(-1, "password hash length not 32");
}

$user = lookup_user_email_addr($email_addr);
if ($user) {
    if ($user->passwd_hash != $passwd_hash) {
        xml_error(-206);
    } else {
        $authenticator = $user->authenticator;
    }
} else {
    $authenticator = random_string();
    $cross_project_id = random_string();
    $now = time();
    $query = "insert into user (create_time, email_addr, name, authenticator, expavg_time, send_email, show_hosts, cross_project_id, passwd_hash) values($now, '$email_addr', '$user_name', '$authenticator', unix_timestamp(), 1, 1, '$cross_project_id', '$passwd_hash')";
    $result = mysql_query($query);
    if (!$result) {
        xml_error(-137);
    }
    
    if(defined('INVITE_CODES')) {
        error_log("New account '$new_name' created using invitation code '$invite_code'");
    }
}

echo " <account_out>\n";
echo "   <authenticator>$authenticator</authenticator>\n";
echo "</account_out>\n";

?>

