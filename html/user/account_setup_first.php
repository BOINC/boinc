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
    To use the defaults settings,
    scroll to the bottom and click OK.
";
$global_prefs = default_prefs_global();
global_prefs_update($user, $global_prefs);
$project_prefs = default_prefs_project();
project_prefs_update($user, $project_prefs);

echo "<form action=account_setup_first_action.php>
    <table cellpadding=6>
";
prefs_form_global($user, $global_prefs);
prefs_form_email($project_prefs);
venue_form($user);

echo "<tr><td><br></td><td><input type=submit value=\"OK\"></td></tr>
    </table>
    </form>\n
";

page_tail();

?>
