<?php

include_once("db.inc");
include_once("util.inc");
include_once("login.inc");
include_once("prefs.inc");

db_init();

$user = get_user_from_cookie();
page_head("Add project");
if ($user == NULL) {
    print_login_form();
} else {
    $prefs = prefs_parse($user->prefs);
    prefs_project_parse_form($project);

    $i = project_index($prefs, $project->master_url);
    if ($i >= 0) {
        echo "Duplicate project URL\n";
    } else {
        if ($prefs->projects == null) {
            $prefs->projects = array($project);
        } else {
            array_push($prefs->projects, $project);
        }
        prefs_update($user, $prefs);
        print_prefs_display($prefs);
    }
}
echo "<p>\n";
page_tail();

?>
