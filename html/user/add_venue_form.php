<?php
    require_once("../inc/db.inc");
    require_once("../inc/util.inc");
    require_once("../inc/prefs.inc");

    db_init();

    $user = get_logged_in_user();

    $venue = get_str("venue");
    check_venue($venue);
    $subset = get_str("subset");
    check_subset($subset);

    $x = subset_name($subset);
    page_head("Add $x preferences for $venue");
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
