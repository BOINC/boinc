<?php
    // show a result

    require_once("db.inc");
    require_once("util.inc");
    require_once("result.inc");

    db_init();
    $resultid = $_GET["resultid"];
    page_head("Result");
    $user = get_logged_in_user();
    $r = mysql_query("select * from result where id=$resultid");
    $result = mysql_fetch_object($r);
    mysql_free_result($r);
    if (!$result) {
        echo "No such result";
        exit();
    }
    //if ($result->userid != $user->id) {
    //    echo "No access";
    //    exit();
    //}
    show_result($result);
    page_tail();
?>
