<?php

require_once("../inc/db.inc");
require_once("../inc/xml.inc");

db_init();

$nonce = process_user_text($_GET["nonce"]);
$email_addr = process_user_text($_GET["email_addr"]);

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

$result = mysql_query(
    "insert into tentative_user (nonce, email_addr, confirmed) values ('$nonce', '$email_addr', 0)"
);

if ($result) {
    $subject = "Confirm ".PROJECT." account";
    $body = "Click to confirm account:
        ".URL_BASE."am_confirm.php?nonce=$nonce
    ";
    $headers = "";
    mail($email_addr, $subject, $body, $headers);
    success();
} else {
    error("database error");
}

?>
