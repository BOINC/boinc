<?php
    require_once("util.inc");
    require_once("db.inc");

    parse_str(getenv("QUERY_STRING"));

    db_init();

    $first = 1;

    print_page_header("Results");

    $query = "select * from result";
    $english_query = "Show all results";
    $count_query = "";

    if (strlen($result_state) && $result_state != 0) {
        $query = append_sql_query( $query, "state = $result_state", $first );
        $count_query = append_sql_query( $count_query, "state = $result_state", $first );
        $english_query = append_sql_query( $english_query, "state is ".res_state_string($result_state), $first );
        $rstate = $result_state;
        $first = 0;
    } else {
        $rstate = 0;
    }

    if (strlen($wu_id)) {
        $query = append_sql_query( $query, "workunitid = $wu_id", $first );
        $count_query = append_sql_query( $count_query, "workunitid = $wu_id", $first );
        $english_query = append_sql_query( $english_query, "workunit is ".wu_name_by_id($wu_id), $first );
        $first = 0;
    }

    if (strlen($host_id)) {
        $query = append_sql_query( $query, "hostid = $host_id", $first );
        $count_query = append_sql_query( $count_query, "hostid = $host_id", $first );
        $english_query = append_sql_query( $english_query, "host is ".host_name_by_id($host_id), $first );
        $first = 0;
    }

    if (strlen($res_batch)) {
        $query = append_sql_query( $query, "batch = $res_batch", $first );
        $count_query = append_sql_query( $count_query, "batch = $res_batch", $first );
        $english_query = append_sql_query( $english_query, "batch number is $res_batch", $first );
        $first = 0;
    }

    if (strlen($exit_status)) {
        $query = append_sql_query( $query, "exit_status = $exit_status", $first );
        $count_query = append_sql_query( $count_query, "exit_status = $exit_status", $first );
        $english_query = append_sql_query( $english_query, "exit status is $exit_status", $first );
        $first = 0;
    }

    if (strlen($nresults)) {
        $results_to_show = $nresults;
    } else {
        $results_to_show = 5;
    }

    if (strlen($show_more)) {
        $start_at = $last_pos;
    } else {
        $start_at = 0;
    }

    printf(
        "<form method=get action=result.php>\n"
        . "Result State: <select name=result_state>\n"
        . "<option value=\"0\"" . ($rstate == 0 ? "selected" : "") . "> All\n"
    );
    for( $i=1;$i<=6;$i++ ) {
        printf( "<option value=\"$i\"" . ($rstate == $i ? "selected" : "") . ">" . res_state_string($i) . "\n" );
    }
    printf(
        "</select>\n"
        . "<p>\n"
        . "Result in Batch: <input name=res_batch value=\"$res_batch\" type=text size=10>\n"
        . "<p>\n"
        . "Result has Exit Code: <input name=exit_status value=\"$exit_status\" type=text size=10>\n"
        . "<p>\n"
        . "Number of Results to Show: <input name=nresults value=\"$results_to_show\" type=text size=10>\n"
        . "<p>\n"
    );
    print_checkbox("Show XML Docs", "show_xml_docs", $show_xml_docs);
    print_checkbox("Show Result stderr", "show_stderr", $show_stderr);
    print_checkbox("Show Times", "show_times", $show_times);
    printf( "<input type=hidden name=last_pos value=\"" . ($results_to_show+$start_at) . "\">\n" );
    print_submit("Query","new_query");
    print_submit("Show More","show_more");

    printf( "</form>\n" );

    echo "<p>Query is: <b>$english_query</b><p>";

    print_query_count("select count(*) as cnt from result" . $count_query, $results_to_show, $start_at);

    $result = mysql_query($query);
    while (($res = mysql_fetch_object($result)) && ($results_to_show > 0)) {
        if ($start_at <= 0) {
            show_result($res,$show_xml_docs,$show_stderr,$show_times);
            $results_to_show--;
        } else {
            $start_at--;
        }
    }

    print_page_end();
?>
