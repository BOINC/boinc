<?php

include_once("db.inc");
include_once("prefs.inc");

parse_str(getenv("QUERY_STRING"));

if ($prefsid) {
    db_init();
    $result = mysql_query("select * from prefs where id=$prefsid");
    $prefs = mysql_fetch_object($result);
    mysql_free_result($result);
    parse_prefs_xml($prefs);
    echo "<pre>".htmlspecialchars($prefs->xml_doc)."</pre>";
    echo "Edit preferences:";
} else {
    $prefs->id = 0;
    $prefs->name = "default preferences";
    $prefs->dont_run_on_batteries = 1;
    $prefs->dont_run_if_user_active = 1;
    $prefs->confirm_before_connecting = 0;
    echo "Create new preferences:";
}


prefs_form($prefs);

?>
