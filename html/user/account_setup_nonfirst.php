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
    <br>
    You can control how your resources
    (computer time and disk space) are divided
    among ".PROJECT." and the other BOINC-based projects
    in which you participate.
    You do this by assigning a <b>resource share</b> to each project.
    The resources allocated to a project are proportional to its resource share.
    The default resource share is 100.
";
$prefs = prefs_parse($user->project_prefs);
prefs_form_resource($prefs, "account_setup_nonfirst_done.php");
page_tail();

?>
