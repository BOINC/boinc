<?php
    require_once("util.inc");
    require_once("user.inc");
    require_once("team.inc");

    db_init();

    page_head("Top teams");
    $result = mysql_query("select * from team order by expavg_credit desc, total_credit desc");
    start_table();
    row1("Teams", 5);
    team_table_start();
    $i = 1;
    while ($team = mysql_fetch_object($result)) {
        show_team_row($team, $i);
        $i++;
    }
    mysql_free_result($result);
    echo "</table>\n<p>\n";
    page_tail();
?>
