<?php
    require_once("util.inc");
    require_once("db.inc");
    require_once("user.inc");
    require_once("host.inc");

    db_init();
    $user = get_logged_in_user();
    $hostid = $HTTP_GET_VARS["hostid"];
    $result = mysql_query("select * from host where id = $hostid");
    $host = mysql_fetch_object($result);
    mysql_free_result($result);
    if (!$host) {
        echo "Couldn't find computer";
        exit();
    }

    $private = false;
    if ($user && $user->id == $host->userid) {
        $private = true;
    }
    page_head("Computer summary", $user);
    show_host($host, $private);
    page_tail();
?>
