<?php
    require_once("util.inc");
    require_once("db.inc");
    require_once("user.inc");
    require_once("host.inc");

    db_init();
    $user = get_logged_in_user();
    $hostid = $HTTP_GET_VARS["hostid"];
    page_head("Computer stats");

    $result = mysql_query("select * from host where id = $hostid");
    $host = mysql_fetch_object($result);
    mysql_free_result($result);

    if ($host) {
        if ($host->userid != $user->id) {
            echo "Not your computer\n";
        } else {
            show_host($host, true);
        }
    } else {
        echo "Couldn't find host or user.<p>";
    }
    page_tail();
?>
