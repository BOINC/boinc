<?php

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

include_once("../inc/db.inc");
include_once("../inc/util.inc");

db_init();

$user = get_logged_in_user();

if ($user) {
    check_tokens($user->authenticator);
    session_start();
    session_destroy();
    setcookie('auth', "", time());
    page_head("Logged out");
    echo "You are now logged out";
    page_tail();
} else {
    error_page("not logged in");
}

?>
