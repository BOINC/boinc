<?php
    require_once("util.inc");
    require_once("user.inc");

    db_init();
    page_head("Top teams");
    $result = mysql_query("select * from team order by expavg_credit desc");
    team_table_start();
    while ($team = mysql_fetch_object($result)) {
        show_team_row($team);
    }
    echo "</table>\n";
    page_tail();
?>
