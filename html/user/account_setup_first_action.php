<?php

include_once("db.inc");
include_once("util.inc");
include_once("prefs.inc");

db_init();

$user = get_logged_in_user();

// TODO: consolidate the three DB updates into one

$prefs = prefs_parse_global($user->global_prefs);
prefs_global_parse_form($prefs);
global_prefs_update($user, $prefs);

$prefs = prefs_parse_project($user->project_prefs);
prefs_email_parse_form($prefs);
project_prefs_update($user, $prefs);

venue_parse_form($user);
venue_update($user);

Header("Location: account_setup_first_download.php");

?>
