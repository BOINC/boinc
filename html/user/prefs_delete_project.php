<?php

include_once("db.inc");
include_once("util.inc");
include_once("login.inc");
include_once("prefs.inc");

db_init();

$user = get_user_from_cookie();
page_head("Edit project preferences");
if ($user == NULL) {
    print_login_form();
} else {
    $prefs = prefs_parse($user->prefs);
    parse_str(getenv("QUERY_STRING"));
    $i = project_index($prefs, $master_url);
    array_splice($prefs->projects, i, 1);
    prefs_update($user, $prefs);
    echo "Project $master_url deleted";
    prefs_form_projects($prefs);
    echo "<br>";
    echo "<a href=prefs.php>Preferences</a>\n";
}
echo "<p>\n";
page_tail();

?>
