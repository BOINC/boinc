<?php
    require_once("db.inc");
    require_once("util.inc");
    require_once("prefs.inc");

    db_init();

    $user = get_logged_in_user();

    $subset = $_GET["subset"];
    page_head(subset_name($subset)." preferences");
	if ($_GET['updated']) {
	    echo '<p style="color: red">Your preference has been updated and will take effect the next time BOINC starts, or you may manually retrieve the preference in the BOINC software.</p>';
	}
    if ($subset == "global") {
        print_prefs_display_global($user);
    } else {
        print_prefs_display_project($user);
    }
    page_tail();

?>
