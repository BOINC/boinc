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
    prefs_global_parse_form($prefs);
    global_prefs_update($user, $prefs);
    echo "<table width=780><tr><td>";
    echo "</td></tr></table>";
    print_prefs_display($user);
}
echo "<p>\n";
page_tail();

?>
