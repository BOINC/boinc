<?php

// "web service" for verifying that a user CPID matches an account key

require_once("../inc/db.inc");

db_init();

$cpid = process_user_text($_GET["cpid"]);;
$authenticator = process_user_text($_GET["authenticator"]);;

if (!$cpid || !$authenticator) {
    echo "<error>missing argument</error>\n";
    exit();
}

$user = lookup_user_auth($authenticator);

if (!$user) {
    echo "<error>bad authenticator</error>\n";
    exit();
}

$x = $user->cross_project_id.$user->email_addr;
if (md5($x) == $cpid) {
    echo "<success></success>\n";
} else {
    echo "<error>bad CPID</error>\n";
}

?>
