<?php
    require_once("util.inc");
    require_once("db.inc");
    require_once("user.inc");
    require_once("host.inc");

    db_init();
    $hostid = $HTTP_GET_VARS["hostid"];
    if ($hostid) {
        page_head("Host stats");

        $result = mysql_query("select * from host where id = $hostid");
        $host = mysql_fetch_object($result);
        mysql_free_result($result);

        if ($host) {
            show_host($host, false);
        } else {
            echo "Couldn't find host.<p>";
        }
    } else {
        echo "Couldn't find host.<p>";
    }
    page_tail();
?>
