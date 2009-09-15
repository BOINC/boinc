<?php

require_once("../inc/util_ops.inc");

$user = get_logged_in_user_ops();
if ($user) {
    clear_cookie('auth', true);
    admin_page_head("Logged out");
    admin_page_tail();
} else {
    error_page_ops("not logged in");
}

?>
