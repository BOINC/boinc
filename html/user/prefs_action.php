<?php
    include_once("db.inc");
    include_once("util.inc");
    db_init();
    $user = get_user_from_cookie();
    if (!user) {
        print_login_form();
        exit();
    }

    parse_str(getenv("QUERY_STRING"));

    $xml = "<preferences>\n";
    if ($dont_run_on_batteries) {
        $xml = $xml."<dont_run_on_batteries/>\n";
    }
    if ($dont_run_if_user_active) {
        $xml = $xml."<dont_run_if_user_active/>\n";
    }
    if ($confirm_before_connecting) {
        $xml = $xml."<confirm_before_connecting/>\n";
    }
    $xml = $xml."</preference>\n";

    $x = time();
    if ($prefsid) {
        $result = mysql_query("select * from prefs where id=$prefsid");
        $prefs = mysql_fetch_object($result);
        mysql_free_result($result);
        if (!$prefs || $prefs->userid != $user->id) {
            echo "You don't own those preferences.";
            exit();
        }

        $query = "update prefs set modified_time=$x, name='$prefs_name', xml_doc='$xml' where id=$prefsid";
        $result = mysql_query($query);
        if ($result) {
            echo "Preferences modified!";
        } else {
            echo "error: ".mysql_error();
        }
    } else {
        $query = "insert into prefs set userid=$user->id, create_time=$x, modified_time = $x, name='$prefs_name', xml_doc='$xml'";
        $result = mysql_query($query);
        if ($result) {
            echo "Preferences created!";
        } else {
            echo "error: ".mysql_error();
        }
    }

?>

