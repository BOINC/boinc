<?php
    require_once("db.inc");
    require_once("util.inc");
    require_once("prefs.inc");

    db_init();

    $user = get_logged_in_user();

    $subset = $_GET["subset"];
    page_head(subset_name($subset)." preferences");
	if ($_GET['updated']) {
	    echo "<p style='color: red'>
            Your preferences have been updated
            and will take effect the next time your computer communicates
            with the project.
            You may manually retrieve the preferences
            using the BOINC client's \"Get Preferences\" command.
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
