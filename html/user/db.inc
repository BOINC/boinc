<?php

// database-related functions.
// Presentation code (HTML) shouldn't be here

function db_init() {
    $retval = mysql_connect();
    if (!$retval) {
        echo "Unable to connect to database - please try again later";
        exit();
    }
    $db_name = parse_config("<db_name>");
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

?>
