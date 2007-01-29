<?php

include_once("../inc/db.inc");
include_once("../inc/util.inc");

if (isset($_COOKIE['auth'])) {
    check_tokens($_COOKIE['auth']);
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
