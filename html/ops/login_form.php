<?php

require_once("../inc/util_ops.inc");

$user = get_logged_in_user_ops();
if ($user) error_page_ops("already logged in");
admin_page_head("Log in");
print_login_form_ops('index.php');
admin_page_tail();
?>
