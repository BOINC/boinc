<?php {
    require_once("util_ops.inc");
    require_once("db_ops.inc");

    function append_sql_query($original,$addition,$first) {
        if ($first == 1) {
            return $original . " where " . $addition;
        } else {
            return $original . " and " . $addition;
        }
    }

    db_init();

    parse_str(getenv("QUERY_STRING")); // 'TODO: remove';
    $q = build_sql_query();

    if (strlen($nresults)) {
        $entries_to_show = $nresults;
    } else {
        $entries_to_show = 10;
    }
    $page_entries_to_show = $entries_to_show;

    if (strlen($last_pos)) {
        $start_at = $last_pos;
    } else {
        $start_at = 0;
    }

    page_head($table);

    $count = $q->count();

    if ($count < $start_at + $entries_to_show) {
        $entries_to_show = $count - $start_at;
    }

    $last = $start_at + $entries_to_show;

    $main_query = $q->get_select_query($entries_to_show, $start_at);

    echo "<p>Query: <b>$main_query</b><p>\n";
    echo "
        <p>$count database entries match the query.
        Displaying $start_at to $last.<p>
    ";

    $urlquery = $q->urlquery;
    echo "<table><tr><td width=100>";
    if ($start_at) {
        $prev_pos = $start_at - $page_entries_to_show;
        if ($prev_pos < 0) {
            $prev_pos = 0;
        }
        echo "
            <a href=db_action.php?table=$table&query=$urlquery&last_pos=$prev_pos&detail=$detail>Previous $page_entries_to_show</a><br>
        ";
    }
    echo "</td><td width=100>";
    if ($last < $count) {
        echo "
            <a href=db_action.php?table=$table&query=$urlquery&last_pos=$last&detail=$detail>Next $entries_to_show</a><br>
        ";
    }
    echo "</td></tr></table>";
    if ($table == "result") {
        echo "<a href=result_summary.php?query=$urlquery>Summary</a> |";
    }
    if ($detail == "high") {
        echo "
            <a href=db_action.php?table=$table&query=$urlquery&detail=low>Less detail</a>
        ";
    }
    if ($detail == "low") {
        echo "
            <a href=db_action.php?table=$table&query=$urlquery&detail=high>More detail</a>
        ";
    }

    echo " | <a href=index.php>Return to main admin page</a>\n";
    echo "<p>\n";
    if ($table == "host") {
        if ($show_aggregate) {
            $result = mysql_query("select sum(d_total) as tot_sum, sum(d_free) as free_sum, " . "sum(m_nbytes) as tot_mem from host" . $query);
            $disk_info = mysql_fetch_object($result);
            $dt = $disk_info->tot_sum/(1024*1024*1024);
            $df = $disk_info->free_sum/(1024*1024*1024);
            $mt = $disk_info->tot_mem/(1024*1024);
            echo "<p>\n
                Sum of total disk space on these hosts:
                $dt GB
                <p>
                Sum of available disk space on these hosts:
                $df GB
                <p>
                Sum of memory on these hosts:
                $mt MB
                <p>
            ";
        }
    }
    $result = mysql_query($main_query);
    if ($result) {
        if ($detail == "low") {
            start_table();
            switch($table) {
            case "result":
                result_short_header();
                break;
            case "host":
                host_short_header();
                break;
            case "app_version":
                app_version_short_header();
                break;
            case "workunit":
                workunit_short_header();
                break;
            }
        }
        while ($res = mysql_fetch_object($result)) {
            if ($detail == "low") {
                switch ($table) {
                case "result":
                    show_result_short($res);
                    break;
                case "host":
                    show_host_short($res);
                    break;
                case "app_version":
                    show_app_version_short($res);
                    break;
                case "workunit":
                    show_workunit_short($res);
                    break;
                }
            } else {
                switch ($table) {
                case "platform":
                    show_platform($res);
                    break;
                case "app":
                    show_app($res);
                    break;
                case "app_version":
                    show_app_version($res, $hide_xml_docs);
                    break;
                case "host":
                    show_host($res);
                    break;
                case "workunit":
                    show_workunit($res, $hide_xml_docs);
                    break;
                case "result":
                    show_result($res, $hide_xml_docs, $hide_stderr, $hide_times);
                    break;
                case "team":
                    show_team($res);
                    break;
                case "user":
                    show_user($res);
                    break;
                }
            }
        }
        if ($detail == "low") {
            end_table();
        }
    } else {
        echo "<h2>No results found</h2>";
    }

    page_tail();
} ?>
