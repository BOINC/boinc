<?php

include_once("db.inc");
include_once("util.inc");
include_once("prefs.inc");

db_init();

$user = get_logged_in_user();

$subset = $_GET["subset"];
$venue = $_GET["venue"];

if ($subset == "global") {
    $main_prefs = prefs_parse_global($user->global_prefs);
    $main_prefs->$venue = null;
    global_prefs_update($user, $main_prefs);
} else {
    $main_prefs = prefs_parse_project($user->project_prefs);
    $main_prefs->$venue = null;
    project_prefs_update($user, $main_prefs);
}
Header("Location: prefs.php?subset=$subset");

?>
