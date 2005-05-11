<?php
    require_once("../inc/db.inc");
    require_once("../inc/util.inc");
if (isset($_GET["cols"])) {
    require_once("../inc/prefs_col.inc");
} else {
    require_once("../inc/prefs.inc");
}

    db_init();

    $user = get_logged_in_user();

    $subset = $_GET["subset"];
    page_head(subset_name($subset)." preferences");
	if (isset($_GET['updated'])) {
	    echo "<p style='color: red'>
            Your preferences have been updated.
            They will take effect when your computer communicates
            with ".PROJECT." or
            you issue the \"Update\" command from the BOINC client.
            </p>
        ";
	}
    if ($subset == "global") {
        print_prefs_display_global($user);
    } else {
        print_prefs_display_project($user);
    }
    page_tail();

?>
