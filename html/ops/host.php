<?php
    require_once("util.inc");
    require_once("db.inc");

    parse_str(getenv("QUERY_STRING"));

    db_init();

    $first = 1;

    print_page_header("Hosts");

    $query = "";
    $english_query = "Show all hosts";

    if (strlen($id)) {
        $query = append_sql_query( $query, "id = $id", $first );
        $english_query = append_sql_query( $english_query, "id is $id", $first );
        $first = 0;
    }
    if (strlen($user_id)) {
        $query = append_sql_query( $query, "userid = $user_id", $first );
        $english_query = append_sql_query( $english_query, "user is ".user_name_by_id($user_id), $first );
        $first = 0;
    }
    printf(
        "<form method=get action=host.php>\n"
        . "<input type=checkbox name=show_aggregate"
        . (strlen($show_aggregate) ? " checked" : "") . ">"
        . "Show Aggregate Information\n"
        . "<p>\n"
        . "<input type=submit value=\"Query\">\n"
        . "</form>\n"
    );

    if ($show_aggregate) {
        $result = mysql_query("select sum(d_total) as tot_sum, "
            . "sum(d_free) as free_sum, "
            . "sum(m_nbytes) as tot_mem "
            . "from host" . $query);
        $disk_info = mysql_fetch_object($result);
        printf(
            "<p>"
            . "Sum of total disk space on these hosts: "
            . $disk_info->tot_sum/(1024*1024*1024) . " GB"
            . "<p>"
            . "Sum of available disk space on these hosts: "
            . $disk_info->free_sum/(1024*1024*1024) . " GB"
            . "<p>"
            . "Sum of memory on these hosts: " . $disk_info->tot_mem/(1024*1024) . " MB"
            . "<p>"
        );
    }

    echo "<p>Query is: <b>$english_query</b><p>";

    $result = mysql_query("select * from host" . $query);
    while ($host = mysql_fetch_object($result)) {
        show_host($host);
    }

    print_page_end();
?>
