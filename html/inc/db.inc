<?php

require_once('../inc/util.inc');

// database-related functions.
// Presentation code (HTML) shouldn't be here

function db_init_aux() {
    $config = get_config();
	$user = parse_config($config, "<db_user>");
	$pass = parse_config($config, "<db_passwd>");
	$host = parse_config($config, "<db_host>");
    if ($host == null) {
        $host = "localhost";
    }
    $retval = mysql_pconnect($host, $user, $pass);
    if (!$retval) {
        return 1;
    }
    $db_name = parse_config($config, "<db_name>");
    if(!mysql_select_db($db_name)) {
        return 2;
    }

    return 0;
}

function lookup_user_id($id) {
    $result = mysql_query("select * from user where id=$id");
    if ($result) {
        $user = mysql_fetch_object($result);
        mysql_free_result($result);
        return $user;
    }
    return null;
}

function lookup_host($id) {
    $result = mysql_query("select * from host where id=$id");
    if ($result) {
        $host = mysql_fetch_object($result);
        mysql_free_result($result);
        return $host;
    }
    return null;
}

function lookup_team($id) {
    $result = mysql_query("select * from team where id=$id");
    if ($result) {
        $team = mysql_fetch_object($result);
        mysql_free_result($result);
        return $team;
    }
    return null;
}

function lookup_wu($id) {
    $result = mysql_query("select * from workunit where id=$id");
    if ($result) {
        $wu = mysql_fetch_object($result);
        mysql_free_result($result);
        return $wu;
    }
    return null;
}

function lookup_app($id) {
    $result = mysql_query("select * from app where id=$id");
    if ($result) {
        $app = mysql_fetch_object($result);
        mysql_free_result($result);
        return $app;
    }
    return null;
}

// escape a string for MySQL "like"
//
function escape_pattern($str) {
    $str = str_replace('_', '\\\\_', $str);
    $str = str_replace('%', '\\\\%', $str);
    return $str;
}

?>
