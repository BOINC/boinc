<?php
    require_once("../inc/util_ops.inc");
    require_once("../inc/db_ops.inc");
    echo "<!--\$Id$ -->\n";

    function append_sql_query($original,$addition,$first) {
        if ($first == 1) {
            return $original . " where " . $addition;
        } else {
            return $original . " and " . $addition;
        }
    }

    db_init();

    $detail = null;

    parse_str(getenv("QUERY_STRING"));
    $q = new SqlQueryString();
    $q->process_form_items();

    $nresults = $_GET['nresults'];
    if (strlen($nresults)) {
        $entries_to_show = $nresults;
    } else {
        $entries_to_show = 10;
    }
    $page_entries_to_show = $entries_to_show;

    $last_pos = $_GET['last_pos'];
    if (strlen($last_pos)) {
        $start_at = $last_pos;
    } else {
        $start_at = 0;
    }

    admin_page_head($table);

    $count = $q->count();

    if ($count < $start_at + $entries_to_show) {
        $entries_to_show = $count - $start_at;
    }

    $last = $start_at + $entries_to_show;

    $main_query = $q->get_select_query($entries_to_show, $start_at);

    echo "<p>Query: <b>$main_query</b><p>\n";
    
    $start_1_offset = $start_at + 1;
    echo "
        <p>$count records match the query.
        Displaying $start_1_offset to $last.<p>
    ";

    $url = $q->get_url("db_action.php");
    if ($detail) {
        $url .= "&detail=$detail";
    }

    //echo "<hr>$url<hr><br>\n";
    if ($start_at || $last < $count) {
        echo "<table border=1><tr><td width=100>";
        if ($start_at) {
            $prev_pos = $start_at - $page_entries_to_show;
            if ($prev_pos < 0) {
                $prev_pos = 0;
            }
            echo "
                <a href=$url&last_pos=$prev_pos&nresults=$entries_to_show>Previous $page_entries_to_show</a><br>
            ";
        }
        echo "</td><td width=100>";
        if ($last < $count) {
            echo "
                <a href=$url&last_pos=$last&nresults=$entries_to_show>Next $entries_to_show</a><br>
            ";
        }
        echo "</td></tr></table>";
    }

    if ($table == "result") {
        $url = $q->get_url("result_summary.php");
        echo "<a href=$url>Summary</a> |";
    }
    if ($detail == "high") {
        $url = $q->get_url("db_action.php")."&detail=low";
        echo "
            <a href=$url>Less detail</a>
        ";
    }
    if ($detail == "low") {
        $url = $q->get_url("db_action.php")."&detail=high";
        echo "
            <a href=$url>More detail</a>
        ";
    }

    echo " | <a href=index.php>Return to main admin page</a>\n";
    echo "<p>\n";
    if ($table == "host") {
        $show_aggregate = $_GET['show_aggregate'];
        if ($show_aggregate) {
            $query = "select sum(d_total) as tot_sum, sum(d_free) as free_sum, sum(m_nbytes) as tot_mem from host";
            $clauses = $_GET['clauses'];
            if ($clauses) {
                echo "$clauses<br>";
                $clauses = stripslashes(urldecode($clauses));
                echo "$clauses<br>";
                $query = "$query WHERE $clauses";
            }
            $result = mysql_query($query);
            $disk_info = mysql_fetch_object($result);
            $dt = $disk_info->tot_sum/(1024*1024*1024);
            $df = $disk_info->free_sum/(1024*1024*1024);
            $mt = $disk_info->tot_mem/(1024*1024);
            $dt = round($dt, 2);
            $df = round($df, 2);
            $mt = round($mt, 2);
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
                    show_app_version($res);
                    break;
                case "host":
                    show_host($res);
                    break;
                case "workunit":
                    show_workunit($res);
                    break;
                case "result":
                    show_result($res);
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

    admin_page_tail();
?>
