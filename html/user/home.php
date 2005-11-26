<?php

require_once("../inc/util.inc");
require_once("../inc/user.inc");
require_once("../inc/db.inc");
require_once("../inc/forum.inc");

// show the home page of whoever's logged in

db_init();
$user = get_logged_in_user();
$user = getForumPreferences($user);
page_head("Your account");

if (get_str("new_acct", true)) {
    echo "
        <p>
        Welcome to ".PROJECT.".
        View and edit your account preferences using the links below.
    ";
}
if (get_str("via_web", true)) {
    echo "
        <p>
        If you have not already done so,
        <a href=http://boinc.berkeley.edu/download.php>download BOINC client software</a>.
    ";
}
echo "<p>\n";

show_user_page_private($user);
project_user_page_private($user);

page_tail();

?>
