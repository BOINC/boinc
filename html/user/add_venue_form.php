<?php
    require_once("db.inc");
    require_once("util.inc");
    require_once("prefs.inc");

    $authenticator = init_session();
    db_init();

    $user = get_user_from_auth($authenticator);
    require_login($user);

    $venue = $_GET["venue"];
    $subset = $_GET["subset"];

    $x = subset_name($subset);
    page_head("Add $x preferences for $venue");
    echo "<h2>Add $x preferences for $venue</h2>";
    echo "<form action=add_venue_action.php>
        <input type=hidden name=venue value=$venue>
        <input type=hidden name=subset value=$subset>
    ";
    start_table();
    if ($subset == "global") {
        $prefs = default_prefs_global();
        prefs_form_global($user, $prefs, null);
    } else {
        $prefs = default_prefs_project();
        prefs_form_resource($prefs);
        prefs_form_project($prefs);
    }
    row2("","<input type=submit value=\"Add preferences\">");
    end_table();
    echo "</form>\n";
    page_tail();

?>
