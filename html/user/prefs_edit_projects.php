<?php

include_once("db.inc");
include_once("util.inc");
include_once("login.inc");
include_once("prefs.inc");

db_init();

$user = get_user_from_cookie();
page_head("Edit Project Prefrences");
if ($user == NULL) {
    print_login_form();
} else {
    $prefs = prefs_parse($user->prefs);
    prefs_form_projects($prefs);
    echo "<br>";
    echo "<a href=prefs.php>Back to preferences</a>\n";
}
echo "<p>\n";
page_tail();

?>
