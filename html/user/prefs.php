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
    $result = mysql_query("select * from prefs where userid=$user->id");
    echo "Existing preference sets (click to view or edit):<br>";
    while ($prefs = mysql_fetch_object($result)) {
        echo "<a href=prefs_edit.php?prefsid=$prefs->id>$prefs->name</a>";
    }
    mysql_free_result($result);
    echo "<br><br><a href=prefs_edit.php>Create new preferences</a>\n";
}

?>
