<?php

require_once("../inc/util.inc");
require_once("../inc/user.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/forum.inc");

// show the home page of logged-in user

$user = get_logged_in_user();
BoincForumPrefs::lookup($user);
$user = get_other_projects($user);

$init = isset($_COOKIE['init']);
$via_web = isset($_COOKIE['via_web']);
if ($via_web) {
    clear_cookie('via_web');
}

if ($init) {
    clear_cookie('init');
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

start_table();
echo "<tr><td valign=top>";
start_table();
show_user_info_private($user);
show_user_stats_private($user);

if (file_exists("../project/donations.inc")) {
    require_once("../project/donations.inc");
    if (function_exists('show_user_donations_private')) {
        show_user_donations_private($user);
    }
}
end_table();
show_other_projects($user, true);
project_user_page_private($user);
echo "</td><td valign=top>";
start_table();
show_community_private($user);
end_table();

echo "</td></tr></table>";

page_tail();

?>
