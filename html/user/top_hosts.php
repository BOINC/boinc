<?php
    require_once("../inc/cache.inc");
    $sort_by = $_GET["sort_by"];
    if (!$sort_by) $sort_by = "expavg_credit";
    $offset = $_GET["offset"];
    if (!$offset) $offset=0;

    // don't cache offsets over 1000.  prevents DOS attack
    //
    if ($offset < 1000) {
        $cache_args = "sort_by=$sort_by&offset=$offset";
        start_cache(3600, $cache_args);
    }

    require_once("../inc/util.inc");
    require_once("../inc/db.inc");
    require_once("../inc/host.inc");

    $n = 10;

    db_init();
    page_head("Top computers", null, null, false);
    if ($sort_by == "total_credit") {
        $sort_clause = "total_credit desc, total_credit desc";
    } else {
        $sort_clause = "expavg_credit desc, total_credit desc";
    }
    $result = mysql_query("select * from host order by $sort_clause limit $n offset $offset");
    host_table_start("Top computers", false, true);
    $i = $offset+1;
    while ($host = mysql_fetch_object($result)) {
        show_host_row($host, $i, false, true);
        $i++;
    }
    mysql_free_result($result);
    echo "</table>\n";
    if ($offset > 0) {
        $new_offset = $offset - $n;
        echo "<a href=top_hosts.php?sort_by=$sort_by&offset=$new_offset>Last $n</a> | ";

    }
    $new_offset = $offset + $n;
    echo "<a href=top_hosts.php?sort_by=$sort_by&offset=$new_offset>Next $n</a>";
    page_tail();

    if ($offset < 1000) {
        end_cache($cache_args);
    }
?>
