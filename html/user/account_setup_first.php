<?php

include_once("../inc/db.inc");
include_once("../inc/util.inc");
include_once("../inc/prefs.inc");
include_once("../inc/translation.inc");

$authenticator = init_session();
db_init();

$user = get_user_from_auth($authenticator);
if ($user == NULL) {
    print_login_form();
    exit();
}

page_head(tr(AC_FIRST_TITLE));
echo "
    <br>".tr(AC_FIRST_CONTROL)."<br>".tr(AC_FIRST_DEFAULT)."<p>
";
$global_prefs = default_prefs_global();
global_prefs_update($user, $global_prefs);

if (strlen($user->project_prefs) == 0) {
    $project_prefs = default_prefs_project();
    project_prefs_update($user, $project_prefs);
}

echo "<form action=account_setup_first_action.php>
    <table cellpadding=6>
";
prefs_form_global($user, $global_prefs);

prefs_form_privacy($user);
venue_form($user);

echo "<tr><td><br></td><td><input type=submit value=\"OK\"></td></tr>
    </table>
    </form>\n
";

page_tail();

?>
