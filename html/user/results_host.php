<?php
    // show recent results for a host

    require_once("util.inc");
    require_once("result.inc");

    db_init();
    $hostid = $_GET["hostid"];
    page_head("Results for host");
    result_table_start();
    $i = 1;
    $result = mysql_query("select * from result where hostid=$hostid order by id desc limit 20");
    while ($res = mysql_fetch_object($result)) {
        show_result_row($res, $i);
        $i++;
    }
    mysql_free_result($result);
    echo "</table>\n";
    page_tail();
?>
