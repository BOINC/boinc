<?php

// database-related functions.
// Presentation code (HTML) shouldn't be here

function db_init() {
    $retval = mysql_pconnect();
    if (!$retval) {
        exit();
    }
    $fp = fopen("db_name", "r");
    $db_name = fgets($fp, 1024);
    $db_name = rtrim($db_name);
    mysql_select_db($db_name);
    fclose($fp);
    return $db_name;
}

function lookup_user_auth($auth) {
    $result = mysql_query("select * from user where authenticator='$auth'");
    if ($result) {
        $user = mysql_fetch_object($result);
        mysql_free_result($result);
        return $user;
    } else {
        return 0;
    }
}

?>
