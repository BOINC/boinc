<?php

include_once("../inc/db.inc");
include_once("../inc/util.inc");
include_once("../inc/prefs.inc");
include_once("../inc/translation.inc");

db_init();

$user = get_logged_in_user();
page_head(tr(AC_NONFIRST_TITLE));
echo "
    <h3>".tr(AC_SETUP_TITLE)."</h3>
";
$project_prefs = prefs_parse_project($user->project_prefs);
echo "<form action=account_setup_nonfirst_action.php>
    <table cellpadding=6>
";
prefs_form_resource($project_prefs);

venue_form($user);

echo "<tr><td><br></td><td><input type=submit value=\"OK\"></td></tr>
    </table>
    </form>\n
";

page_tail();

?>
