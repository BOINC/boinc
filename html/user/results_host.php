<?php
    // show recent results for a host

    require_once("db.inc");
    require_once("util.inc");
    require_once("result.inc");

    $results_per_page = 20;

    db_init();
    $hostid = $_GET["hostid"];
    $offset = $_GET["offset"];
    if (!$offset) $offset=0;
    page_head("Results for host");
    echo "<h3>Results for host</h3>\n";
    result_table_start(true, false);
    $i = 1;
    $result = mysql_query("select * from result where hostid=$hostid order by id desc limit $results_per_page offset $offset");
    while ($res = mysql_fetch_object($result)) {
        show_result_row($res, true, false);
        $i++;
    }
    mysql_free_result($result);
    echo "</table>\n";
    if ($i > $results_per_page) {
	$offset = $offset+$results_per_page;
        echo "
            <br><center><a href=results_host.php?hostid=$hostid&offset=$offset>Next $results_per_page results</a></center>
        ";
    }

    page_tail();
?>
