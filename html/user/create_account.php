<?php

// RPC handler for account creation

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../inc/xml.inc");

db_init();

xml_header();

$config = get_config();
if (parse_bool($config, "disable_account_creation") || defined('INVITE_CODES')) {
    echo "<account_out>\n";
    echo "<error_num>-208</error_num>\n";
    echo "</account_out>\n";
    exit();
}

$email_addr = get_str("email_addr");
$email_addr = process_user_text(strtolower($email_addr));
$passwd_hash = process_user_text(get_str("passwd_hash"));
$user_name = process_user_text(get_str("user_name"));

if (!is_valid_email_addr($email_addr)) {
    echo "<account_out>\n";
    echo "   <error_num>-205</error_num>\n";
    echo "</account_out>\n";
    exit();
}

if (strlen($passwd_hash) != 32) {
    echo "<account_out>\n";
    echo "   <error_num>-206</error_num>\n";
    echo "</account_out>\n";
    exit();
}

$user = lookup_user_email_addr($email_addr);
$bad = false;
if ($user) {
    if ($user->passwd_hash != $passwd_hash) {
        $bad = true;
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
        $bad = true;
    }
}

if ($bad) {
    echo "<account_out>\n";
    echo "   <error_num>-207</error_num>\n";
    echo "</account_out>\n";
} else {
    echo " <account_out>\n";
    echo "   <authenticator>$authenticator</authenticator>\n";
    echo "</account_out>\n";
}

?>

