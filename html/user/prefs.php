<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/prefs.inc");

db_init();

$user = get_logged_in_user();

$subset = get_str("subset");
$columns = get_int("cols", true);
$updated = get_int("updated", true);

page_head(subset_name($subset)." preferences");
if (isset($updated)) {
	echo "<p style='color: red'>
        Your preferences have been updated.
        Client-related preferences
        will take effect when your computer communicates
        with ".PROJECT." or
        you issue the \"Update\" command from the BOINC client.
        </p>
    ";
}
if ($subset == "global") {
    print_prefs_display_global($user, $columns);
} else {
    print_prefs_display_project($user, $columns);
}
page_tail();

?>
