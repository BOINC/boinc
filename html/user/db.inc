<?php

require_once('util.inc');

// database-related functions.
// Presentation code (HTML) shouldn't be here

function db_init($pathMod='') {
    if (project_is_stopped()) {
        echo "Project is shut down for maintenance - please try again later\n";
        exit();
    }
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

function lookup_host($id) {
    $result = mysql_query("select * from host where id='$id'");
    if ($result) {
        $host = mysql_fetch_object($result);
        mysql_free_result($result);
        return $host;
    }
    return null;
}

function lookup_team($id) {
    $result = mysql_query("select * from team where id='$id'");
    if ($result) {
        $team = mysql_fetch_object($result);
        mysql_free_result($result);
        return $team;
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
