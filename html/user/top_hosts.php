<?php
    require_once("db.inc");
    require_once("util.inc");
    require_once("host.inc");

    $n = 10;
    $offset = $_GET["offset"];
    if (!$offset) $offset=0;

    db_init();
    page_head("Top computers");
    $sort_by = $_GET["sort_by"];
    if ($sort_by == "total_credit") {
        $sort_clause = "total_credit desc, total_credit desc";
    } else {
        $sort_clause = "expavg_credit desc, total_credit desc";
    }
    $result = mysql_query("select * from host order by $sort_clause limit $n offset $offset");
    host_table_start("Top computers", false);
    $i = $offset+1;
    while ($host = mysql_fetch_object($result)) {
        show_host_row($host, $i, false);
        $i++;
    }
    mysql_free_result($result);
    echo "</table>\n";
    if ($offset > 0) {
        $new_offset = $offset - $n;
        echo "<a href=top_hosts?sort_by=$sort_by&offset=$new_offset>Last $n</a> | ";

    }
    $new_offset = $offset + $n;
    echo "<a href=top_hosts?sort_by=$sort_by&offset=$new_offset>Next $n</a>";
    page_tail();
?>
