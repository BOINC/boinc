<?php

require_once('util.inc');

// database-related functions.
// Presentation code (HTML) shouldn't be here

function db_init($pathMod='') {
    $retval = mysql_pconnect();
    if (!$retval) {
        echo "Unable to connect to database - please try again later";
        exit();
    }
    $db_name = parse_config("<db_name>", $pathMod);
    if(!mysql_select_db($db_name)) {
        echo "Unable to select database - please try again later";
        exit();
    }

    return 0;
}

function lookup_user_auth($auth) {
    $result = mysql_query("select * from user where authenticator='$auth'");
    if ($result) {
        $user = mysql_fetch_object($result);
        mysql_free_result($result);
        return $user;
    }
    return null;
}

function lookup_user_id($id) {
    $result = mysql_query("select * from user where id='$id'");
    if ($result) {
        $user = mysql_fetch_object($result);
        mysql_free_result($result);
        return $user;
    }
    return null;
}

function lookup_wu($id) {
    $result = mysql_query("select * from workunit where id='$id'");
    if ($result) {
        $wu = mysql_fetch_object($result);
        mysql_free_result($result);
        return $wu;
    }
    return null;
}

function lookup_app($id) {
    $result = mysql_query("select * from app where id='$id'");
    if ($result) {
        $app = mysql_fetch_object($result);
        mysql_free_result($result);
        return $app;
    }
    return null;
}

?>
