<?php
    require_once("util.inc");
    require_once("user.inc");

function host_table_start() {
    echo <<<EOT
        <table border=1 cellpadding=6>
        <tr><th>Owner</th>
        <th>Total credit</th>
        <th>Recent average credit</th>
        <th>CPU type</th>
        </tr>
EOT;
}

function show_host_row($host) {
    $result = mysql_query("select * from user where id = $host->userid");
    $user = mysql_fetch_object($result);
    mysql_free_result($result);
    echo "<tr>
        <td><a href=show_user.php?id=$user->id>$user->name</a></td>
        <td>$host->total_credit</td>
        <td>$host->expavg_credit</td>
        <td>$host->p_vendor $host->p_model</td>
        </tr>";
}

    $max_hosts_display = 100;
    db_init();
    page_head("Top hosts");
    $result = mysql_query("select * from host order by expavg_credit desc limit $max_hosts_display");
    host_table_start();
    while (($host = mysql_fetch_object($result)) && $max_hosts_display > 0) {
        show_host_row($host);
        $max_hosts_display--;
    }
    mysql_free_result($result);
    echo "</table>\n";
    page_tail();
?>
