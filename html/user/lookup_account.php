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
if (!$user) {
    echo "<account_out>";
    echo "<error_num>-161</error_num>\n";
    echo "</account_out>";
    exit();
}

echo "<account_out>";
echo "<authenticator>$user->authenticator</authenticator>";
echo "</account_out>";

?>

