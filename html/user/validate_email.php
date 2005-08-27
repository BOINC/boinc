<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");

db_init();
$user = get_logged_in_user();

$x = process_user_text(get_str("x"));
$u = process_user_text(get_int("u"));

$user = lookup_user_id($u);
if (!$user) {
    error_page("No such user.\n");
}

$x2 = md5($user->email_addr.$user->authenticator);
if ($x2 != $x) {
    error_page("Error in URL data - can't validate email address");
}

$result = mysql_query("update user set email_validated=1 where id=$user->id");
if (!$result) {
    error_page("Database update failed - please try again later.");
}

page_head("Validate email address");
echo "
    The email address of your account has been validated.
";
page_tail();

?>
