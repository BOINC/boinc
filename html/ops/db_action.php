<?php
    require_once("util_ops.inc");
    require_once("db.inc");

function append_sql_query($original,$addition,$first) {
    if ($first == 1) {
        return $original . " where " . $addition;
    } else {
        return $original . " and " . $addition;
    }
}

    db_init();

    parse_str(getenv("QUERY_STRING"));

    $first = 1;

    if (strlen($clauses)) {
        $query = append_sql_query( $query, $clauses, $first );
        $first = 0;
    }

    if (strlen($id)) {
        $query = append_sql_query( $query, "id = $id", $first );
        $first = 0;
    }

    if (strlen($plat_id)) {
        $query = append_sql_query( $query, "platformid = $plat_id", $first );
        $first = 0;
    }

    if (strlen($app_id)) {
        $query = append_sql_query( $query, "appid = $app_id", $first );
        $first = 0;
    }

    if (strlen($wu_id)) {
        $query = append_sql_query( $query, "workunitid = $wu_id", $first );
        $first = 0;
    }

    if (strlen($hostid)) {
        $query = append_sql_query( $query, "hostid = $hostid", $first );
        $first = 0;
    }

    if (strlen($userid)) {
        $query = append_sql_query( $query, "userid = $userid", $first );
        $first = 0;
    }

    if (strlen($team_id)) {
        $query = append_sql_query( $query, "teamid = $team_id", $first );
        $first = 0;
    }

    if (strlen($received_time)) {
        $query = append_sql_query( $query, "received_time > $received_time", $first );
        $first = 0;
    }

    if (strlen($result_server_state) && $result_server_state>0) {
        $query = append_sql_query( $query, "server_state = $result_server_state", $first );
        $first = 0;
    }

    if (strlen($result_outcome) && $result_outcome>0) {
        $query = append_sql_query( $query, "outcome = $result_outcome", $first );
        $first = 0;
    }

    if (strlen($result_client_state) && $result_client_state>0) {
        $query = append_sql_query( $query, "client_state = $result_client_state", $first );
        $first = 0;
    }

    if (strlen($sort_by)) {
        $query = $query . " order by $sort_by desc";
        $first = 0;
    }

    if (strlen($nresults)) {
        $entries_to_show = $nresults;
    } else {
        $entries_to_show = 10;
    }

    if (strlen($last_pos)) {
        $start_at = $last_pos;
    } else {
        $start_at = 0;
    }

    page_head($table);


    $count_query = "select count(*) as cnt from ".$table." ".$query;
    $result = mysql_query($count_query);
    $res = mysql_fetch_object($result);
    $count = $res->cnt;

    if ($count < $start_at + $entries_to_show) {
        $entries_to_show = $count - $start_at;
    }

    $last = $start_at + $entries_to_show;

    if ($start_at) {
        $main_query = "select * from ".$table." ".$query. " limit ".$start_at.",".$entries_to_show;
    } else {
        $main_query = "select * from ".$table." ".$query. " limit ".$entries_to_show;
    }

    echo "<p>Query: <b>$main_query</b><p>\n";
    echo "
        <p>$count database entries match the query.
        Displaying $start_at to $last.<p>
    ";

    $urlquery = urlencode($query);
    if ($last < $count) {
        echo "
            <a href=db_action.php?table=$table&query=$urlquery&last_pos=$last&detail=$detail>Next $entries_to_show</a>
        ";
    }
    if ($detail == "high") {
        echo "
            | <a href=db_action.php?table=$table&query=$urlquery&detail=low>Less detail</a>
        ";
    }
    if ($detail == "low") {
        echo "
            | <a href=db_action.php?table=$table&query=$urlquery&detail=high>More detail</a>
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
    if ($detail == "low") {
        start_table();
        switch($table) {
            case "result": result_short_header(); break;
            case "host":   host_short_header(); break;
        }
    }
    $result = mysql_query($main_query);
    while ($res = mysql_fetch_object($result)) {
        if ($detail == "low") {
            switch ($table) {
            case "result":      show_result_short($res);                                      break;
            case "host":        show_host_short($res);                                        break;
            }
        } else {
            switch ($table) {
            case "platform":    show_platform($res);                                          break;
            case "app":         show_app($res);                                               break;
            case "app_version": show_app_version($res, $hide_xml_docs);                       break;
            case "host":        show_host($res);                                              break;
            case "workunit":    show_workunit($res, $hide_xml_docs);                          break;
            case "result":      show_result($res, $hide_xml_docs, $hide_stderr, $hide_times); break;
            case "team":        show_team($res);                                              break;
            case "user":        show_user($res);                                              break;
            }
        }
    }
    if ($detail == "low") {
        end_table();
    }

    page_tail();
?>
