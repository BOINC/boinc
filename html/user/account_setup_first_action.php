<?php

include_once("db.inc");
include_once("util.inc");
include_once("prefs.inc");

$authenticator = init_session();
db_init();

$user = get_user_from_auth($authenticator);
if ($user == NULL) {
    print_login_form();
    exit();
}

// TODO: consolidate the three DB updates into one

$prefs = prefs_parse($user->prefs);
prefs_global_parse_form($prefs);
global_prefs_update($user, $prefs);

prefs_email_parse_form($prefs);
project_prefs_update($user, $prefs);

venue_parse($user);
venue_update($user);

Header("Location: account_setup_first_download.php");

?>
