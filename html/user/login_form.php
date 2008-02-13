<?php
require_once("../inc/db.inc");
require_once("../inc/util.inc");

$next_url = get_str("next_url", true);

$user = get_logged_in_user(false);

page_head("Log in/out");
print_login_form_aux($next_url, $user);

page_tail();
?>
