<?php

require_once("../inc/util_ops.inc");

$user = get_logged_in_user_ops();
if ($user) admin_error_page("already logged in");
admin_page_head("Log in");

$next_url = sanitize_local_url(get_str('next_url', true));

if (strlen($next_url) == 0) $next_url = "index.php";

print_login_form_ops($next_url);
admin_page_tail();
?>
