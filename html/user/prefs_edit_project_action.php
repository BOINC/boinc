<?php

include_once("db.inc");
include_once("util.inc");
include_once("login.inc");
include_once("prefs.inc");

db_init();

$user = get_user_from_cookie();
page_head("Preferences");
if ($user == NULL) {
    print_login_form();
} else {
    $prefs = prefs_parse($user->project_prefs);
    parse_str(getenv("QUERY_STRING"));
    prefs_project_parse_form($prefs);
    project_prefs_update($user, $prefs);
    print_prefs_display($prefs);
}
echo "<p>\n";
page_tail();

?>
