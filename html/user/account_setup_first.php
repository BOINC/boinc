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

page_head("Account configuration");
echo "
    <h3>Account configuration</h3>
    Thank you for allowing ".PROJECT." to use part of your computer power.
    <br>
    You control when and how your computer is used.
    <br>
    We suggest using the defaults -
    just go to the bottom and click OK.
";
$prefs = default_prefs();
global_prefs_update($user, $prefs);
project_prefs_update($user, $prefs);

prefs_form_global($user, $prefs, "account_setup_first_project.php");
page_tail();

?>
