<?php
    // show a result

    require_once("../inc/db.inc");
    require_once("../inc/util.inc");
    require_once("../inc/result.inc");

    db_init();
    $resultid = $_GET["resultid"];
    page_head("Result");
    $r = mysql_query("select * from result where id=$resultid");
    $result = mysql_fetch_object($r);
    mysql_free_result($r);
    if (!$result) {
        echo "No such result";
        exit();
    }
    show_result($result);
    page_tail();
?>
