<?php

include_once("db.inc");
include_once("util.inc");
include_once("prefs.inc");

db_init();

$authenticator = init_session();
$user = get_user_from_auth($authenticator);
if ($user == NULL) {
    print_login_form();
    exit();
}

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
    prefs_global_parse_form($prefs);
    if ($venue) $main_prefs->$venue = $prefs;
    else $main_prefs = $prefs;
    project_prefs_update($user, $main_prefs);
}
Header("Location: prefs.php?subset=$subset");

?>
