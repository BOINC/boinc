<?php

include_once("../inc/db.inc");
include_once("../inc/util.inc");

$user = get_logged_in_user();

if ($user) {
    check_tokens($user->authenticator);
    clear_cookie('auth');
    page_head("Logged out");
    echo "You are now logged out";
    page_tail();
} else {
    error_page("not logged in");
}

?>
