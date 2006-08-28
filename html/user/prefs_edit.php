<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
include_once("../inc/db.inc");
include_once("../inc/util.inc");
include_once("../inc/prefs.inc");

db_init();

$user = get_logged_in_user();

$action = get_str("action", true);
$subset = get_str("subset");
$venue = get_str("venue", true);
$columns = get_str("cols", true);
$c = $columns?"&cols=$columns":"";
check_subset($subset);

// lets see if we got some data to process
if ($action) {
    // Yes we got some form data now process them
    if ($subset == "global") {
    //process the submitted data for general prefs
        $main_prefs = prefs_parse_global($user->global_prefs);
        if ($venue) $prefs = $main_prefs->$venue;
        else $prefs = $main_prefs;
        $error = prefs_global_parse_form($prefs);
        if ($error != false) {
        // We encountered an error during parsing lets display the form again
            $title = "Edit ".subset_name($subset)." preferences";
            if ($venue) $title = "$title for $venue";
            page_head($title);
            $x = $venue?"&venue=$venue":"";

            echo PREFS_FORM_DESC1;
            echo PREFS_FORM_ERROR_DESC;

            print_prefs_form("edit", $subset, $venue, $user, $prefs, $columns, $error);
        } else {
        // No error occured so lets update the values and go back to prefs page
            if ($venue) $main_prefs->$venue = $prefs;
            else $main_prefs = $prefs;
            global_prefs_update($user, $main_prefs);
            Header("Location: prefs.php?subset=$subset&updated=1$c");
        }
    } else {
    // process the submited data for project specific prefs
        $main_prefs = prefs_parse_project($user->project_prefs);
        if ($venue) $prefs = $main_prefs->$venue;
        else $prefs = $main_prefs;

        $project_error = prefs_project_parse_form($prefs);
        $error = prefs_resource_parse_form($prefs);
        if ($error != false || $project_error != false) {
        // We encountered an error during parsing lets display the form again
            $title = "Edit ".subset_name($subset)." preferences";
            if ($venue) $title = "$title for $venue";
            page_head($title);
            $x = $venue?"&venue=$venue":"";

            echo PREFS_FORM_ERROR_DESC;

            print_prefs_form("edit", $subset, $venue, $user, $prefs, $columns, $error, $project_error);
        } else {
        // No error occured so lets update the values and go back to prefs page
            if ($venue) {
                $main_prefs->$venue = $prefs;
            } else {
                $main_prefs = $prefs;
                prefs_privacy_parse_form($user);
            }

            project_prefs_update($user, $main_prefs);

            if (!$venue) {
                venue_parse_form($user);
                venue_update($user);
            }
        Header("Location: prefs.php?subset=$subset&updated=1$c");
        }
    }
} else {
// We got no data to process so lets display the current values
    $title = "Edit ".subset_name($subset)." preferences";
    if ($venue) $title = "$title for $venue";
    page_head($title);
    $x = $venue?"&venue=$venue":"";

    if ($subset == "global") {
        echo PREFS_FORM_DESC1;
        $prefs = prefs_parse_global($user->global_prefs);
        if ($venue) {
            $prefs = $prefs->$venue;
        }
    } else {
        $prefs = prefs_parse_project($user->project_prefs);
        if ($venue) {
            $prefs = $prefs->$venue;
        }
    }
    print_prefs_form("edit", $subset, $venue, $user, $prefs, $columns);
}
echo "<a href=prefs.php?subset=$subset$x$c>Back to preferences</a>\n";
page_tail();

?>
