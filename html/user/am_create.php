<?php

require_once("../inc/db.inc");
require_once("../inc/xml.inc");
require_once("../inc/email.inc");

db_init();

$nonce = process_user_text($_GET["nonce"]);
$email_addr = process_user_text($_GET["email_addr"]);
$email_addr = strtolower($email_addr);
$acct_mgr_name = process_user_text($_GET["acct_mgr_name"]);

xml_header();

function reply($x) {
    echo "<am_create_reply>
    $x
</am_create_reply>
";
    exit();
}

function error($x) {
    reply("<error>$x</error>");
}

function success() {
    reply("<success/>");
}

if (strlen($nonce)==0) {
    error("missing nonce ID");
}
if (strlen($email_addr)==0) {
    error("missing email addr");
}

$config = get_config();
if (parse_bool($config, "disable_account_creation")) {
    error("account creation disabled");
}

$user = lookup_user_email_addr($email_addr);
if (!$user) {
    $user = lookup_user_munged_email($email_addr);
}

if ($user) {
    error("account with that email already exists");
}

$query = "insert into tentative_user (nonce, email_addr, confirmed) values ('$nonce', '$email_addr', 0)";
$result = mysql_query($query);

if ($result) {
    $subject = "Confirm ".PROJECT." account";
    $body = "
".PROJECT." (".MASTER_URL.")
has been requested by $acct_mgr_name
to create an account with email address $email_addr.
If you wish to create this account, visit the following URL:

".URL_BASE."am_confirm.php?nonce=$nonce

If you didn't initiate this request, ignore this message.
    ";
    $headers = "";
    mail($email_addr, $subject, $body, $headers);
    success();
} else {
    error("database error: ".mysql_error());
}

?>
