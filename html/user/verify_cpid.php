<?php

// "web service" for verifying that a user CPID matches an account key

require_once("../inc/db.inc");
require_once("../inc/xml.inc");

db_init();

xml_header();

function reply($x) {
    echo "<reply>\n$x</reply>\n";
}

$cpid = process_user_text($_GET["cpid"]);;
$authenticator = process_user_text($_GET["authenticator"]);;

if (!$cpid || !$authenticator) {
    reply("<error>missing argument</error>\n");
    exit();
}

$user = lookup_user_auth($authenticator);

if (!$user) {
    reply("<error>bad authenticator</error>\n");
    exit();
}

$x = $user->cross_project_id.$user->email_addr;
if (md5($x) == $cpid) {
    reply( "<success></success>\n");
} else {
    reply( "<error>bad CPID</error>\n");
}

?>
