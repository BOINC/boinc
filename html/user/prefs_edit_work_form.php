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
    page_head("Edit Work Preferences");
    $prefs = prefs_parse($user->prefs);
    prefs_form_work($user, $prefs);
    echo "<a href=prefs.php>Back to preferences</a>\n";
}
echo "<p>\n";
page_tail();

?>
