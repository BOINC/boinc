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
page_head("Edit global preferences", $user);
$prefs = prefs_parse($user->global_prefs);
echo "<h3>Edit global preferences</h3>
    These preferences apply to all the BOINC projects
    in which you participate.
    <br>If you participate in multiple BOINC projects,
    edit your preferences only one project's web site;
    <br>otherwise edits may be overwritten.
";

prefs_form_global($user, $prefs, "prefs.php");
echo "<a href=prefs.php>Back to preferences</a>\n";
page_tail();

?>
