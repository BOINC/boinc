<?php
    require_once("util.inc");
    require_once("db.inc");

    parse_str(getenv("QUERY_STRING"));

    db_init();

    $first = 1;

    print_page_header("Workunits");

    $query = "select * from workunit";
    $count_query = "";
    $english_query = "Show all workunits";

    if (strlen($id)) {
        $query = append_sql_query( $query, "id = $id", $first );
        $count_query = append_sql_query( $count_query, "id = $id", $first );
        $english_query = append_sql_query( $english_query, "id is $id", $first );
        $first = 0;
    }

    if (strlen($app_id)) {
        $query = append_sql_query( $query, "appid = $app_id", $first );
        $count_query = append_sql_query( $count_query, "appid = $app_id", $first );
        $english_query = append_sql_query( $english_query, "application is " . app_name_by_id($app_id), $first );
        $first = 0;
    }

    if (strlen($wu_batch)) {
        $query = append_sql_query( $query, "batch = $wu_batch", $first );
        $count_query = append_sql_query( $count_query, "batch = $wu_batch", $first );
        $english_query = append_sql_query( $english_query, "batch number is $wu_batch", $first );
        $first = 0;
    }

    if (strlen($nres_done)) {
        $query = append_sql_query( $query, "nresults_done = $nres_done", $first );
        $count_query = append_sql_query( $count_query, "nresults_done = $nres_done", $first );
        $english_query = append_sql_query( $english_query, "number of results done is $nres_done", $first );
        $first = 0;
    }

    if (strlen($nres_fail)) {
        $query = append_sql_query( $query, "nresults_fail = $nres_fail", $first );
        $count_query = append_sql_query( $count_query, "nresults_fail = $nres_fail", $first );
        $english_query = append_sql_query( $english_query, "number of results failed is $nres_fail", $first );
        $first = 0;
    }

    if (strlen($nres_unsent)) {
        $query = append_sql_query( $query, "nresults_unsent = $nres_unsent", $first );
        $count_query = append_sql_query( $count_query, "nresults_unsent = $nres_unsent", $first );
        $english_query = append_sql_query( $english_query, "number of results unsent is $nres_unsent", $first );
        $first = 0;
    }

    if (strlen($show_more)) {
        $start_at = $last_pos;
    } else {
        $start_at = 0;
    }
    if (strlen($nwus)) {
        $wus_to_show = $nwus;
    } else {
        $wus_to_show = 5;
    }

    printf(
        "<form method=get action=workunit.php>\n"
        . "Workunits in Batch Number: <input name=wu_batch value=\"$wu_batch\" type=text size=10>\n"
        . "<p>"
        . "Number of Workunits to Show: <input name=nwus value=\"$wus_to_show\" type=text size=10>\n"
        . "<p>"
        . "Number of Results Done: <input name=nres_done value=\"$nres_done\" type=text size=10>\n"
        . "<p>"
        . "Number of Results Failed: <input name=nres_fail value=\"$nres_fail\" type=text size=10>\n"
        . "<p>"
        . "Number of Results Unsent: <input name=nres_unsent value=\"$nres_unsent\" type=text size=10>\n"
        . "<p>"
    );

    print_checkbox("Show XML Docs", "show_xml_docs", $show_xml_docs);
    printf( "<input type=hidden name=last_pos value=\"" . ($wus_to_show+$start_at) . "\">\n" );
    print_submit("Query","new_query");
    print_submit("Show More","show_more");

    printf( "</form>\n" );

    echo "<p>Query is: <b>$english_query</b><p>";

    print_query_count("select count(*) as cnt from workunit" . $count_query, $wus_to_show, $start_at );

    $result = mysql_query($query);
    while (($workunit = mysql_fetch_object($result)) && ($wus_to_show > 0)) {
        if ($start_at <= 0) {
            show_workunit($workunit,$show_xml_docs);
            $wus_to_show--;
        } else {
            $start_at--;
        }
    }

    print_page_end();
?>
