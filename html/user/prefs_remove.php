<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

include_once("../inc/db.inc");
include_once("../inc/util.inc");
include_once("../inc/prefs.inc");

db_init();

$user = get_logged_in_user();
check_tokens($user->authenticator);

$subset = get_str("subset");
$venue = get_str("venue");
$confirmed = get_str("confirmed", true);
$columns = get_int("cols", true);
$c = $columns?"&cols=$columns":"";

if ($confirmed) {
    if ($subset == "global") {
        $main_prefs = prefs_parse_global($user->global_prefs);
        $main_prefs->$venue = null;
        global_prefs_update($user, $main_prefs);
    } else {
        $main_prefs = prefs_parse_project($user->project_prefs);
        $main_prefs->$venue = null;
        project_prefs_update($user, $main_prefs);
    }
    Header("Location: prefs.php?subset=$subset$c");
} else {
    page_head("Confirm delete preferences");
    $tokens = url_tokens($user->authenticator);
    echo "
        <p>
        Are you sure you want to delete your separate ", subset_name($subset),
        " preferences for $venue?
        <br><br>
        <a href=prefs_remove.php?subset=$subset&venue=$venue&confirmed=yes$c$tokens>Yes</a>
        | <a href=prefs.php?subset=$subset$c>Cancel</a>
    ";
    page_tail();
}

?>
