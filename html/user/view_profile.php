<?php
 
require_once("profile.inc");

$userid = $_GET['userid'];

// Check for recommendation or rejection votes.;
if ($_POST['recommend']) {
    process_view_results("recommend", $userid);
    exit();
} else if ($_POST['reject']) {
    process_view_results("reject", $userid);
    exit();
}

show_profile($userid, array_key_exists('verify', $_GET));
?>
