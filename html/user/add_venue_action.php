<?php
    require_once("../inc/db.inc");
    require_once("../inc/util.inc");
    require_once("../inc/prefs.inc");

    db_init();

    $user = get_logged_in_user();

    $venue = get_str("venue");
    check_venue($venue);
    $subset = get_str("subset");
    check_subset($subset);

    if ($subset == "global") {
        $prefs = prefs_parse_global($user->global_prefs);
        prefs_global_parse_form($new_prefs);
        $prefs->$venue = $new_prefs;
        $retval = global_prefs_update($user, $prefs);
    } else {
        $prefs = prefs_parse_project($user->project_prefs);
        prefs_project_parse_form($new_prefs);
        prefs_resource_parse_form($new_prefs);
        $prefs->$venue = $new_prefs;
        $retval = project_prefs_update($user, $prefs);
    }
    if ($retval) {
        Header("Location: prefs.php?subset=$subset");
    } else {
        db_error_page();
    }
?>
