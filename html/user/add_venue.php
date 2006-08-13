<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/prefs.inc");

db_init();

$user = get_logged_in_user();

$action = get_str("action", true);
$subset = get_str("subset");
$venue = get_str("venue");
$columns = get_int("cols", true);
$c = $columns?"&cols=$columns":"";
check_venue($venue);
check_subset($subset);

if ($action) {
    if ($subset == "global") {
        $prefs = prefs_parse_global($user->global_prefs);
        $prefs->$venue = $prefs;
        $error = prefs_global_parse_form($new_prefs);
        if ($error != false) {
            $title = "Edit ".subset_name($subset)." preferences";
            if ($venue) $title = "$title for $venue";
            page_head($title);
            $x = $venue?"&venue=$venue":"";

            echo PREFS_FORM_DESC1;
            echo PREFS_FORM_ERROR_DESC;

            print_prefs_form(
                "add", $subset, $venue, $user, $new_prefs, $columns, $error
            );
        } else {
            $prefs->$venue = $new_prefs;
            global_prefs_update($user, $prefs);
            Header("Location: prefs.php?subset=$subset$c");
        }
    } else {
        $prefs = prefs_parse_project($user->project_prefs);
        $prefs->$venue = $prefs;

        $project_error = prefs_project_parse_form($new_prefs);
        $error = prefs_resource_parse_form($new_prefs);

        if ($error != false || $project_error != false) {
            $title = "Edit ".subset_name($subset)." preferences";
            if ($venue) $title = "$title for $venue";
            page_head($title);
            $x = $venue?"&venue=$venue":"";

            echo PREFS_FORM_ERROR_DESC;

            print_prefs_form(
                "add", $subset, $venue, $user, $new_prefs, $columns,
                $error, $project_error
            );
        } else {
            $prefs->$venue = $new_prefs;
            project_prefs_update($user, $prefs);
            Header("Location: prefs.php?subset=$subset&c");
        }
    }
} else {
    $title = "Add ".subset_name($subset)." preferences for $venue";
    page_head($title);

    if ($subset == "global") {
        $prefs = default_prefs_global();
    } else {
        $prefs = default_prefs_project();
    }
    print_prefs_form("add", $subset, $venue, $user, $prefs, $columns);
}
page_tail();
?>
