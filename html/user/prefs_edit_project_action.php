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
    $prefs = prefs_parse($user->prefs);
    parse_str(getenv("QUERY_STRING"));
    $i = project_index($prefs, $master_url);
    prefs_project_parse_form($project);
    $prefs->projects[$i] = $project;
    prefs_update($user, $prefs);
    print_prefs_display($prefs);
}

?>
