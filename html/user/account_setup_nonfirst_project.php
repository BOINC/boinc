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
page_head("Account setup: graphics", $user);
echo "
    <h3>Account setup: graphics</h3>
    <br>
    ".PROJECT." lets you control the following properties of its graphics.
";
$prefs = null;
prefs_form_project($prefs, "account_setup_nonfirst_done.php");
page_tail();

?>
