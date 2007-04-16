<?php

include_once("../inc/db.inc");
include_once("../inc/util.inc");
include_once("../inc/prefs.inc");

db_init();

$user = get_logged_in_user();

$prefs = prefs_parse_global($user->global_prefs);
prefs_resource_parse_form($prefs);
project_prefs_update($user, $prefs);

Header("Location: account_setup_nonfirst_done.php");

?>
