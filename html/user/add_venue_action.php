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

    if ($subset == "global") {
        $prefs = prefs_parse_global($user->global_prefs);
        prefs_global_parse_form($new_prefs);
        $prefs->$venue = $new_prefs;
        $retval = global_prefs_update($user, $prefs);
    } else {
        $prefs = prefs_parse_project($user->project_prefs);
        prefs_project_parse_form($new_prefs);
        $prefs->$venue = $new_prefs;
        $retval = project_prefs_update($user, $prefs);
    }
    if (retval) {
        Header("Location: prefs.php?subset=$subset");
    } else {
        db_error_page();
    }
?>
