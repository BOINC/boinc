<?php

include_once("../inc/db.inc");
include_once("../inc/util.inc");
include_once("../inc/prefs.inc");

db_init();

$user = get_logged_in_user();

// TODO: consolidate the three DB updates into one

$prefs = prefs_parse_global($user->global_prefs);
prefs_global_parse_form($prefs);
global_prefs_update($user, $prefs);

$prefs = prefs_parse_project($user->project_prefs);
prefs_privacy_parse_form($prefs);
project_prefs_update($user, $prefs);

venue_parse_form($user);
venue_update($user);

Header("Location: account_setup_first_done.php");

?>
