<?php

include_once("../inc/db.inc");
include_once("../inc/util.inc");
include_once("../inc/prefs.inc");

db_init();

$user = get_logged_in_user();

$subset = $_GET["subset"];
$venue = $_GET["venue"];

if ($subset == "global") {
    $main_prefs = prefs_parse_global($user->global_prefs);
    if ($venue) $prefs = $main_prefs->$venue;
    else $prefs = $main_prefs;
    prefs_global_parse_form($prefs);
    if ($venue) $main_prefs->$venue = $prefs;
    else $main_prefs = $prefs;
    global_prefs_update($user, $main_prefs);
} else {
    $main_prefs = prefs_parse_project($user->project_prefs);
    if ($venue) $prefs = $main_prefs->$venue;
    else $prefs = $main_prefs;

    prefs_project_parse_form($prefs);
    prefs_resource_parse_form($prefs);

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
}

Header("Location: prefs.php?subset=$subset&updated=1");

?>
