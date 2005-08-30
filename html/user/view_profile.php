<?php
 
require_once("../inc/forum.inc");
require_once("../inc/profile.inc");
require_once("../inc/text_transform.inc");

db_init();

$userid = get_int('userid');

// Check for recommendation or rejection votes

if (isset($_POST['recommend'])) {
    process_view_results("recommend", $userid);
    exit();
} else if (isset($_POST['reject'])) {
    process_view_results("reject", $userid);
    exit();
}

show_profile($userid);
?>
