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

$prefs = prefs_parse($user->prefs);
prefs_global_parse_form($prefs);
global_prefs_update($user, $prefs);
Header("Location: prefs.php");

?>
