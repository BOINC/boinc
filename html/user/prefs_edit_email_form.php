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
page_head("Edit email options", $user);
parse_str(getenv("QUERY_STRING"));
$prefs = prefs_parse($user->project_prefs);
prefs_form_email($prefs, "prefs.php");
page_tail();

?>
