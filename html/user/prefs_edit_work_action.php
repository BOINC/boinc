<?php

include_once("db.inc");
include_once("util.inc");
include_once("login.inc");
include_once("prefs.inc");

db_init();

$user = get_user_from_cookie();
if ($user == NULL) {
    print_login_form();
} else {
    page_head("Preferences");
    $prefs = prefs_parse($user->prefs);
    prefs_work_parse_form($prefs);
    prefs_update($user, $prefs);
    print_prefs_display($prefs);
}
echo "<p>\n";
page_tail();

?>
