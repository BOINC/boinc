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
page_head("Edit Global Preferences");
$prefs = prefs_parse($user->prefs);
prefs_form_global($user, $prefs);
echo "<a href=prefs.php>Back to preferences</a>\n";
echo "<p>\n";
page_tail();

?>
