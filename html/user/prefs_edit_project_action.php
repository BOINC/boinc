<?php

include_once("db.inc");
include_once("util.inc");
include_once("prefs.inc");

db_init();

$user = get_user_from_cookie();
if ($user == NULL) {
    print_login_form();
    exit();
}

page_head("Preferences");
$prefs = prefs_parse($user->project_prefs);
prefs_project_parse_form($prefs);
project_prefs_update($user, $prefs);
print_prefs_display($user);
echo "<p>\n";
page_tail();

?>
