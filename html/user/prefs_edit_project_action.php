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

no_cache();
$prefs = prefs_parse($user->project_prefs);
prefs_project_parse_form($prefs);
prefs_resource_parse_form($prefs);
prefs_email_parse_form($prefs);
project_prefs_update($user, $prefs);

venue_parse($user);
venue_update($user);

Header("Location: prefs.php");

?>
