<?php {
    require_once("../inc/cache.inc");
    $sort_by = $_GET["sort_by"];
    if (!$sort_by) $sort_by = "expavg_credit";
    $cache_args = "sort_by=$sort_by";
    start_cache(3600, $cache_args);

    require_once("../inc/util.inc");
    require_once("../inc/db.inc");
    require_once("../inc/user.inc");

    if ($sort_by == "total_credit") {
        $sort_order = "total_credit desc, total_credit desc";
    } else {
        $sort_order = "expavg_credit desc, total_credit desc";
    }

    db_init();
    $numusers = 100;
    page_head("Top $numusers users");
    $result = mysql_query("select * from user order by $sort_order limit $numusers");
    row1("Users", 6);
    user_table_start();
    $i = 0;
    while ($user = mysql_fetch_object($result)) {
        show_user_row($user, ++$i);
    }
    echo "</table>\n";
    page_tail();

    end_cache($cache_args);
} ?>
