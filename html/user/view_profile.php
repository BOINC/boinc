<?php
 
require_once("../inc/profile.inc");

db_init();

$userid = get_int('userid');

// Check for recommendation or rejection votes

if ($_POST['recommend']) {
    process_view_results("recommend", $userid);
    exit();
} else if ($_POST['reject']) {
    process_view_results("reject", $userid);
    exit();
}

show_profile($userid);
?>
