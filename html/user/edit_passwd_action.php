<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/user.inc");

db_init();

$user = get_logged_in_user();

$passwd = strtolower(process_user_text(post_str("passwd")));
$passwd2 = strtolower(process_user_text(post_str("passwd2")));

page_head("Change password");

if ($passwd != $passwd2) {
    error_page("passwords are different");
}

$passwd_hash = md5($passwd.$user->email_addr);
$result = mysql_query("update user set passwd_hash='$passwd_hash' where id=$user->id");
if ($result) {
    echo "password changed";
} else {
    echo "
        We can't update your password due to a database problem.
        Please try again later.
    ";
}

page_tail();

?>
