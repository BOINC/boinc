<?php

include_once("db.inc");
include_once("util.inc");
include_once("login.inc");
include_once("prefs.inc");

db_init();

$user = get_user_from_cookie();
page_head("Preferences");
if ($user == NULL) {
    //show_login($user);
    print_login_form();
} else {
    $prefs = prefs_parse($user->prefs);
    print_prefs_display($prefs);
}
echo "<p>\n";
page_tail();

?>
