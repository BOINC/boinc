<?php

include_once("db.inc");
include_once("util.inc");
include_once("prefs.inc");

db_init();

$user = get_logged_in_user();

$subset = $_GET["subset"];
$venue = $_GET["venue"];

$title = "Edit ".subset_name($subset)." preferences";
if ($venue) $title = "$title for $venue";
page_head($title, $user);

echo "<h3>$title</h3>\n";

if ($subset == "global") {
    $prefs = prefs_parse_global($user->global_prefs);
    if ($venue) {
        $prefs = $prefs->$venue;
    }
    echo "
        These preferences apply to all the BOINC projects
        in which you participate.
        <br>If you participate in multiple BOINC projects,
        edit your preferences only one project's web site;
        <br>otherwise edits may be overwritten.
    ";
} else {
    $prefs = prefs_parse_project($user->project_prefs);
    if ($venue) {
        $prefs = $prefs->$venue;
    }
}

$x = $venue?"&\$venue=$venue":"";
echo "<form action=prefs_edit_action.php>
    <input type=hidden name=subset value=$subset>
";
if ($venue) {
    echo "<input type=hidden name=venue value=$venue>\n";
}

start_table();
if ($subset == "global") {
    prefs_form_global($user, $prefs);
} else {
    prefs_form_resource($prefs);
    prefs_form_project($prefs->project_specific);
    if (!$venue) {
        prefs_form_email($prefs);
        venue_form($user);
    }
}

row2("<br>", "<input type=submit value=\"Update preferences\">");
end_table();
echo "</form>\n";

echo "<a href=prefs.php?subset=$subset$x>Back to preferences</a>\n";
page_tail();

?>
