<?php

include_once("db.inc");
include_once("util.inc");
include_once("login.inc");
include_once("prefs.inc");

db_init();

$user = get_user_from_cookie();
if ($user == NULL) {
    print_login_form();
} else {
    echo "Add a project:<br>\n";
    prefs_form_project($project, "prefs_add_project_action.php");
}

?>
