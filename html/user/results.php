<?php
    // show recent results for a host or user

    require_once("db.inc");
    require_once("util.inc");
    require_once("result.inc");

    $results_per_page = 20;

    db_init();
    $hostid = $_GET["hostid"];
    $userid = $_GET["userid"];
    $offset = $_GET["offset"];
    if (!$offset) $offset=0;
    if ($hostid) {
        $type = "host";
        $clause = "hostid=$hostid";
    } else {
        $type = "user";
        $clause = "userid=$userid";
    }
    page_head("Results for $type");
    echo "<h3>Results for $type</h3>\n";
    result_table_start(true, false);
    $i = 1;
    $query = "select * from result where $clause order by id desc limit $results_per_page offset $offset";
    $result = mysql_query($query);
    while ($res = mysql_fetch_object($result)) {
        show_result_row($res, true, false);
        $i++;
    }
    mysql_free_result($result);
    echo "</table>\n";
    if ($i > $results_per_page) {
        $offset = $offset+$results_per_page;
        echo "
            <br><center><a href=results.php?$clause&offset=$offset>Next $results_per_page results</a></center>
        ";
    }

    page_tail();
?>
