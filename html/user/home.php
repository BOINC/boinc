<?php

require_once("../inc/util.inc");
require_once("../inc/user.inc");
require_once("../inc/db.inc");
require_once("../inc/forum.inc");

// show the home page of whoever's logged in

db_init();
$user = get_logged_in_user();
$user = getForumPreferences($user);
$user = get_other_projects($user);

$init = isset($_COOKIE['init']);
$via_web = isset($_COOKIE['via_web']);
if ($via_web) {
    setcookie('via_web', '', time()-3600);
}

if ($init) {
    setcookie('init', '', time()-3600);
    page_head(tra("Welcome to %1", PROJECT));
    echo "<p>".tra("View and edit your account preferences using the links below.")."</p>\n";
    if ($via_web) {
        echo "
            <p> If you have not already done so,
            <a href=\"http://boinc.berkeley.edu/download.php\">download BOINC client software</a>.</p>
        ";
    }
} else {
    page_head("Your account");
}

show_user_page_private($user);
show_other_projects($user, true);
project_user_page_private($user);

page_tail();

?>
