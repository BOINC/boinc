<?php {

    require_once("db.inc");
    require_once("util.inc");
    require_once("user.inc");
    require_once("team.inc");

    db_init();

    page_head("Top teams");
    $sort_by = $_GET["sort_by"];
    if ($sort_by == "total_credit") {
        $sort_by = "total_credit desc, total_credit desc";
    } else {
        $sort_by = "expavg_credit desc, total_credit desc";
    }
    $result = mysql_query("select * from team order by $sort_by");
    start_table();
    row1("Teams", 6);
    team_table_start();
    $i = 1;
    while ($team = mysql_fetch_object($result)) {
        show_team_row($team, $i);
        $i++;
    }
    mysql_free_result($result);
    echo "</table>\n<p>\n";
    page_tail();

} ?>
