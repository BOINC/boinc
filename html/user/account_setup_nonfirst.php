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
page_head("Account setup: resource share", $user);
echo "
    <h3>Account setup</h3>
";
$prefs = prefs_parse($user->project_prefs);
echo "<form action=account_setup_nonfirst_action.php>
    <table cellpadding=6>
";
prefs_form_resource($prefs);

venue_form($user);

echo "<tr><td><br></td><td><input type=submit value=\"OK\"></td></tr>
    </table>
    </form>\n
";

page_tail();

?>
