<?php

require_once("../inc/db.inc");

$nonce = process_user_text($_POST["nonce"]);

$tuser = lookup_tentative_user($nonce);

if (!$tuser) {
    $x = urlencode("nonce not found");
    echo "status=$x\n";
    exit();
}

if (!$tuser->confirmed) {
    echo "status=OK&confirmed=0\n";
    exit();
}

$user = lookup_user_email_addr($tuser->email_addr);

if (!$user) {
    $authenticator = random_string();
    $cross_project_id = random_string();
    $now = time();
    $query = "insert into user (create_time, email_addr, authenticator, cross_project_id) values($now, '$tuser->email_addr', '$authenticator', '$cross_project_id')";
    $result = mysql_query($query);
    $user = lookup_user_auth($authenticator);
}

if (!$user) {
    $x = urlencode("couldn't create user record");
    echo "status=$x\n";
    exit();
}

echo "status=OK&account_key=$user->authenticator\n";

?>
