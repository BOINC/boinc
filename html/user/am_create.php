<?php

require_once("../inc/db.inc");

db_init();

$nonce = process_user_text($_POST["nonce"]);
$email_addr = process_user_text($_POST["email_addr"]);

if (strlen($nonce)==0) {
    echo "status=".urlencode("no nonce ID");
}
if (strlen($email_addr)==0) {
    echo "status=".urlencode("no email addr");
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
    echo "status=OK";
} else {
    echo "status=".urlencode("error");
}

?>
