<?php
    require_once("db.inc");
    require_once("util.inc");
    require_once("host.inc");

    $max_hosts_display = 100;
    db_init();
    page_head("Top computers");
    $sort_by = $_GET["sort_by"];
    if ($sort_by == "total_credit") {
        $sort_by = "total_credit desc, total_credit desc";
    } else {
        $sort_by = "expavg_credit desc, total_credit desc";
    }
    $result = mysql_query("select * from host order by $sort_by limit $max_hosts_display");
    host_table_start("Top computers", false);
    $i = 1;
    while (($host = mysql_fetch_object($result)) && $max_hosts_display > 0) {
        show_host_row($host, $i, false);
        $max_hosts_display--;
        $i++;
    }
    mysql_free_result($result);
    echo "</table>\n";
    page_tail();
?>
