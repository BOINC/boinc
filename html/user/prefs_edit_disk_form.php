<?php

include_once("db.inc");
include_once("util.inc");
include_once("login.inc");
include_once("prefs.inc");

db_init();

$user = get_user_from_cookie();
page_head("Edit Disk Preferences");
if ($user == NULL) {
    print_login_form();
} else {
    $prefs = prefs_parse($user->prefs);
    prefs_form_disk($user, $prefs);
    echo "<a href=prefs.php>Preferences</a>\n";
}
echo "<p>\n";
page_tail();

?>
