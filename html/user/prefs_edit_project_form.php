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
page_head("Edit project preferences", $user);
parse_str(getenv("QUERY_STRING"));
$prefs = prefs_parse($user->project_prefs);

echo "<form action=prefs_edit_project_action.php>
    <table cellpadding=6>
";

prefs_form_resource($prefs);
prefs_form_project($prefs->project_specific);
prefs_form_email($prefs);
venue_form($user);

echo "<tr><td><br></td><td><input type=submit value=\"OK\"></td></tr>
    </table>
    </form>\n
";

page_tail();

?>
