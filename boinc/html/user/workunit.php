<?php
    // show summary of a workunit

    require_once("../inc/db.inc");
    require_once("../inc/result.inc");

    db_init();
    $wuid = $_GET["wuid"];
    page_head("Work unit");
    echo "<h3>Summary of workunit</h3>\n";
    $wu = lookup_wu($wuid);
    if (!$wu) {
        echo "can't find workunit";
        exit();
    }

    $app = lookup_app($wu->appid);

    start_table();
    row2("application", $app->user_friendly_name);
    row2("created", time_str($wu->create_time));
    row2("name", $wu->name);
    row2("granted credit", format_credit($wu->canonical_credit));
    echo "</table>\n";

    echo "<br><br><b>Results:</b>\n";
    result_table_start(false, true, true);
    $result = mysql_query("select * from result where workunitid=$wuid");
    while ($res = mysql_fetch_object($result)) {
        show_result_row($res, false, true, true);
    }
    mysql_free_result($result);
    echo "</table>\n";
    page_tail();
?>
