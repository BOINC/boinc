<?php
    // show summary of a workunit

    require_once("db.inc");
    require_once("util.inc");
    require_once("result.inc");

    db_init();
    $wuid = $_GET["wuid"];
    page_head("Work unit");
    echo "<h3>Summary of workunit</h3>\n";
    $result = mysql_query("select * from workunit where id=$wuid");
    $wu = mysql_fetch_object($result);
    mysql_free_result($result);

    start_table();
    row2("created", time_str($wu->create_time));
    row2("name", $wu->name);
    row2("granted credit", $wu->canonical_credit);
    echo "</table>\n";

    echo "<br><br><b>Results:</b>\n";
    result_table_start(false, true);
    $result = mysql_query("select * from result where workunitid=$wuid");
    while ($res = mysql_fetch_object($result)) {
        show_result_row($res, false, true);
    }
    mysql_free_result($result);
    echo "</table>\n";
    page_tail();
?>
