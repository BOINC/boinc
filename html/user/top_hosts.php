<?php
    require_once("util.inc");
    require_once("host.inc");

    $max_hosts_display = 100;
    db_init();
    page_head("Top computers");
    $result = mysql_query("select * from host order by expavg_credit desc limit $max_hosts_display");
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
