<?php

include_once("db.inc");
include_once("util.inc");
include_once("login.inc");
include_once("prefs.inc");

db_init();

$user = get_user_from_cookie();
page_head("Add project");
if ($user == NULL) {
    print_login_form();
} else {
    prefs_form_project($project, "prefs_add_project_action.php");
}
echo "<p>\n";
page_tail();

?>
