<?php

include_once("db.inc");
include_once("util.inc");
include_once("prefs.inc");

db_init();

$user = get_logged_in_user();
page_head("Account setup: resource share", $user);
echo "
    <h3>Account setup</h3>
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
