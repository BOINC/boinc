<?php

function parse_prefs_xml(&$prefs) {
    if (strstr($prefs->xml_doc, "<dont_run_on_batteries/>")) {
        $prefs->dont_run_on_batteries = 1;
    }
    if (strstr($prefs->xml_doc, "<dont_run_if_user_active/>")) {
        $prefs->dont_run_if_user_active = 1;
    }
    if (strstr($prefs->xml_doc, "<confirm_before_connecting/>")) {
        $prefs->confirm_before_connecting = 1;
    }
}

function prefs_form($prefs) {
    echo   "<form action=prefs_action.php?prefsid=$prefs->id>\n";
    if ($prefs->id) {
        echo   "    <input type=hidden name=prefsid value=$prefs->id>\n";
    }
    echo   "    <table border=1 cellpadding=6>\n";
    echo   "    <tr><td align=right>name for this set of preferences</td><td><input name=prefs_name value='$prefs->name'></td></tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right>don't run if on batteries</td>\n";
    printf("        <td><input type=checkbox name=dont_run_on_batteries %s></td>\n", $prefs->dont_run_on_batteries?"checked":"");
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right>don't run if user is active</td>\n";
    printf("        <td><input type=checkbox name=dont_run_if_user_active %s></td>\n", $prefs->dont_run_if_user_active?"checked":"");
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right>confirm before connecting</td>\n";
    printf("        <td><input type=checkbox name=confirm_before_connecting %s></td>\n", $prefs->confirm_before_connecting?"checked":"");
    echo   "    </tr>\n";
    echo   "    <tr><td><br></td><td><input type=submit value=OK></td></tr>\n";
    echo   "    </table>\n";
    echo   "</form>\n";
}

?>
