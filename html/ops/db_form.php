<?php
    require_once("util.inc");
    require_once("db.inc");

    db_init();

    parse_str(getenv("QUERY_STRING"));

    $first = 1;

    $title = table_title($table);
    page_head($title);
    echo "<form method=get action=db_action.php>\n";
    echo "<p>\n";
    echo "<input type=hidden name=table value=$table>\n";

    start_table();
    if ($table=="platform") {
    } else if ($table=="app") {
    } else if ($table=="app_version") {
        print_checkbox("Show XML Docs", "show_xml_docs", $show_xml_docs);
    } else if ($table=="host") {
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
    } else if ($table=="workunit") {
        print_checkbox("Show XML fields", "show_xml_docs", $show_xml_docs);
    } else if ($table=="result") {
        echo "<tr><td align=right>Server state</td><td> ";
        result_server_state_select();
        echo "</td></tr>\n";
        //print_text_field( "Batch number:", "batch", $batch );
        echo "<tr><td align=right>Outcome</td><td>";
        result_outcome_select();
        echo "</td></tr>\n";
        echo "<tr><td align=right>Client state</td><td>";
        result_client_state_select();
        echo "</td></tr>\n";

        row2("Show XML fields", "<input type=checkbox name=show_xml_docs>");
        row2("Show result stderr", "<input type=checkbox name=show_stderr>");
        row2("Show times", "<input type=checkbox name=show_times>");
        echo "<tr><td align=right>Sort by</td><td>";
        result_sort_select();
        echo "</td></tr>\n";
        echo "<tr><td align=right>Detail level</td><td>";
        echo "<select name=detail>
            <option value=low>low
            <option value=high>high
            </select>
            </td></tr>
        ";

    } else if ($table=="team") {
    } else if ($table=="user") {
    } else {
        echo "Unknown table name\n";
        exit();
    }

    row2("Number of entries to show", "<input name=nresults>");

    row2("", "<input type=submit value=\"OK\">\n");
    end_table();
    echo "</form>\n";

    page_tail();
?>
