<?php
    require_once("util.inc");
    require_once("db.inc");

    db_init();

    function print_header($show) {
        switch($show) {
            case "platform": $title = "Platforms"; break;
            case "app": $title = "Applications"; break;
            case "app_version": $title = "Application Versions"; break;
            case "host": $title = "Hosts"; break;
            case "workunit": $title = "Workunits"; break;
            case "result": $title = "Results"; break;
            case "team": $title = "Teams"; break;
            case "user": $title = "Users"; break;
            default: print_page_header("Database"); return;
        }

        print_page_header($title);
        print_form_header($show);
    }

    parse_str(getenv("QUERY_STRING"));

    $first = 1;

    $query = "from $show";
    $english_query = "Select all from $show";

    if (strlen($id)) {
        $query = append_sql_query( $query, "id = $id", $first );
        $english_query = append_sql_query( $english_query, "id is $id", $first );
        $first = 0;
    }

    if (strlen($plat_id)) {
        $query = append_sql_query( $query, "platformid = $plat_id", $first );
        $english_query = append_sql_query( $english_query, "platform is ".platform_name_by_id($plat_id), $first );
        $first = 0;
    }

    if (strlen($app_id)) {
        $query = append_sql_query( $query, "appid = $app_id", $first );
        $english_query = append_sql_query( $english_query, "application is ".app_name_by_id($app_id), $first );
        $first = 0;
    }

    if (strlen($wu_id)) {
        $query = append_sql_query( $query, "workunitid = $wu_id", $first );
        $english_query = append_sql_query( $english_query, "workunit is ".wu_name_by_id($wu_id), $first );
        $first = 0;
    }

    if (strlen($host_id)) {
        $query = append_sql_query( $query, "hostid = $host_id", $first );
        $english_query = append_sql_query( $english_query, "host is ".host_name_by_id($host_id), $first );
        $first = 0;
    }

    if (strlen($team_id)) {
        $query = append_sql_query( $query, "teamid = $team_id", $first );
        $english_query = append_sql_query( $english_query, "team is " . team_name_by_id($team_id), $first );
        $first = 0;
    }

    if (strlen($result_state) && $result_state != 0) {
        $query = append_sql_query( $query, "state = $result_state", $first );
        $english_query = append_sql_query( $english_query, "state is ".res_state_string($result_state), $first );
        $rstate = $result_state;
        $first = 0;
    } else {
        $rstate = 0;
    }

    if (strlen($batch)) {
        $query = append_sql_query( $query, "batch = $batch", $first );
        $english_query = append_sql_query( $english_query, "batch number is $batch", $first );
        $first = 0;
    }

    if (strlen($nres_done)) {
        $query = append_sql_query( $query, "nresults_done = $nres_done", $first );
        $english_query = append_sql_query( $english_query, "number of results done is $nres_done", $first );
        $first = 0;
    }

    if (strlen($nres_fail)) {
        $query = append_sql_query( $query, "nresults_fail = $nres_fail", $first );
        $english_query = append_sql_query( $english_query, "number of results failed is $nres_fail",$first );
        $first = 0;
    }

    if (strlen($nres_unsent)) {
        $query = append_sql_query( $query, "nresults_unsent = $nres_unsent", $first );
        $english_query = append_sql_query( $english_query, "number of results unsent is $nres_unsent", $first );
        $first = 0;
    }

    if (strlen($exit_status)) {
        $query = append_sql_query( $query, "exit_status = $exit_status", $first );
        $english_query = append_sql_query( $english_query, "exit status is $exit_status", $first );
        $first = 0;
    }

    if (strlen($sort_by)) {
        switch ($sort_by) {
            case 1:
            $query = $query . " order by create_time desc";
            $english_query = append_sql_query( $english_query, "most recent created are listed first", $first );
            $first = 0;
            break;
            case 2:
            $query = $query . " order by sent_time desc";
            $english_query = append_sql_query( $english_query, "most recent sent are listed first", $first );
            $first = 0;
            break;
            case 3:
            $query = $query . " order by received_time desc";
            $english_query = append_sql_query( $english_query, "most recent received are listed first", $first );
            $first = 0;
            break;
        }
    }

    if (strlen($nresults)) {
        $entries_to_show = $nresults;
    } else {
        $entries_to_show = 5;
    }

    if (strlen($show_more)) {
        $start_at = $last_pos;
    } else {
        $start_at = 0;
    }

    print_header($show);

    if ($show=="platform") {
    } else if ($show=="app") {
    } else if ($show=="app_version") {
        print_checkbox("Show XML Docs", "show_xml_docs", $show_xml_docs);
    } else if ($show=="host") {
        print_checkbox("Show Aggregate Information", "show_aggregate", $show_aggregate);
        if ($show_aggregate) {
            $result = mysql_query("select sum(d_total) as tot_sum, sum(d_free) as free_sum, "
                            . "sum(m_nbytes) as tot_mem " . $query);
            $disk_info = mysql_fetch_object($result);
            printf( "<p>\n"
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
    } else if ($show=="workunit") {
        print_text_field( "Workunits in Batch Number:", "batch", $batch );
        print_text_field( "Number of Results Done:", "nres_done", $nres_done );
        print_text_field( "Number of Results Failed:", "nres_fail", $nres_fail );
        print_text_field( "Number of Results Unsent:", "nres_unsent", $nres_unsent );
        print_checkbox("Show XML Docs", "show_xml_docs", $show_xml_docs);
    } else if ($show=="result") {
        printf( "Result State: <select name=result_state>\n"
                . "<option value=\"0\"" . ($rstate == 0 ? "selected" : "") . "> All\n"
            );
        for( $i=1;$i<=6;$i++ ) {
            printf( "<option value=\"$i\"" . ($rstate == $i ? "selected" : "") . ">" . res_state_string($i) . "\n" );
        }
        printf( "</select>\n<p>\n" );
        print_text_field( "Result in Batch Number:", "batch", $batch );
        print_text_field( "Result has Exit Code:", "exit_status", $exit_status );

        print_checkbox("Show XML Docs", "show_xml_docs", $show_xml_docs);
        print_checkbox("Show Result stderr", "show_stderr", $show_stderr);
        print_checkbox("Show Times", "show_times", $show_times);
        printf( "Sort by:<br>\n" );
        print_radio_button("None", "sort_by", "0", $sort_by == "0");
        print_radio_button("Creation Time", "sort_by", "1", $sort_by == "1");
        print_radio_button("Sent Time", "sort_by", "2", $sort_by == "2");
        print_radio_button("Received Time", "sort_by", "3", $sort_by == 3);
        printf("<br>\n");
    } else if ($show=="team") {
    } else if ($show=="user") {
    } else {
        echo "<br><a href=db.php?show=platform>Platform</a>\n";
        echo "<br><a href=db.php?show=app>App</a>\n";
        echo "<br><a href=db.php?show=app_version>App Version</a>\n";
        echo "<br><a href=db.php?show=host>Host</a>\n";
        echo "<br><a href=db.php?show=workunit>Workunit</a>\n";
        echo "<br><a href=db.php?show=result>Result</a>\n";
        echo "<br><a href=db.php?show=team>Team</a>\n";
        echo "<br><a href=db.php?show=user>User</a>\n";
        print_page_end();
        return;
    }

    print_text_field( "Number of Entries to Show:", "nresults", $entries_to_show );
    printf( "<input type=hidden name=last_pos value=\"" . ($entries_to_show+$start_at) . "\">\n" );
    print_form_end();

    echo "<p>Query is: <b>$english_query</b><p>";

    print_query_count("select count(*) as cnt " . $query, $entries_to_show, $start_at);

    $result = mysql_query("select * " . $query);
    while (($res = mysql_fetch_object($result)) && ($entries_to_show > 0)) {
        if ($start_at <= 0) {
            switch ($show) {
                case "platform": show_platform($res); break;
                case "app": show_app($res); break;
                case "app_version": show_app_version($res,$show_xml_docs); break;
                case "host": show_host($res); break;
                case "workunit": show_workunit($res,$show_xml_docs); break;
                case "result": show_result($res,$show_xml_docs,$show_stderr,$show_times); break;
                case "team": show_team($res); break;
                case "user": show_user($res); break;
            }
            $entries_to_show--;
        } else {
            $start_at--;
        }
    }

    print_page_end();
?>
