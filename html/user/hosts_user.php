<?php
    require_once("util.inc");
    require_once("host.inc");

    $max_hosts_display = 100;
    db_init();
    $userid = $_GET["userid"];
    $result = mysql_query("select * from user where id=$userid");
    $user = mysql_fetch_object($result);
    mysql_free_result($result);
    host_table_start("Hosts belonging to $user->name");
    $i = 1;
    $result = mysql_query("select * from host where userid=$userid order by expavg_credit desc limit $max_hosts_display");
    while (($host = mysql_fetch_object($result)) && $max_hosts_display > 0) {
        show_host_row($host, $i);
        $max_hosts_display--;
        $i++;
    }
    mysql_free_result($result);
    echo "</table>\n";
    page_tail();
?>
