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

page_head("Account setup", $user);
echo "
    <h3>Account setup</h3>
    Thank you for letting ".PROJECT." use part of your computer power.
    <br>
    You can control when and how your computer is used.
    <br>
    We suggest using the defaults settings -
    just go to the bottom and click OK.
";
$prefs = default_prefs();
global_prefs_update($user, $prefs);
project_prefs_update($user, $prefs);

prefs_form_global($user, $prefs, "account_setup_first_email.php");
page_tail();

?>
