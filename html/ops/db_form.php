<?php
    require_once("util_ops.inc");
require_once("db_ops.inc");

    db_init();

    parse_str(getenv("QUERY_STRING"));

    function print_detail_field() {
        echo "<tr><td align=right>Detail level</td><td>";
        echo "<select name=detail>
            <option value=low>low
            <option value=high>high
            </select>
            </td></tr>
        ";
    }
    function print_query_field() {
        echo "
            <tr>
            <td align=right>Additional clauses</td>
            <td><input name=clauses></td>
            </tr>
        ";
    }

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
        print_checkbox("Hide XML Docs", "hide_xml_docs", $hide_xml_docs);
        print_detail_field();
    } else if ($table=="host") {
        print_checkbox("Show Aggregate Information", "show_aggregate", $show_aggregate);
        print_detail_field();
    } else if ($table=="workunit") {
        print_detail_field();
        print_query_field();
        echo "<input type=hidden name=sort_by value=id>\n";
    } else if ($table=="result") {
        echo "<tr><td align=right>Server state</td><td> ";
        server_state_select();
        echo "</td></tr>\n";
        //print_text_field( "Batch number:", "batch", $batch );
        echo "<tr><td align=right>Outcome</td><td>";
        outcome_select();
        echo "</td></tr>\n";
        echo "<tr><td align=right>Client state</td><td>";
        client_state_select();
        echo "</td></tr>\n";

        row2("Hide XML fields", "<input type=checkbox name=hide_xml_docs>");
        row2("Hide result stderr", "<input type=checkbox name=hide_stderr>");
        row2("Hide times", "<input type=checkbox name=hide_times>");
        echo "<tr><td align=right>Sort by</td><td>";
        result_sort_select();
        echo "</td></tr>\n";
        print_detail_field();
        print_query_field();

    } else if ($table=="team") {
    } else if ($table=="user") {
    } else {
        echo "Unknown table name\n";
        exit();
    }

    row2("Number of entries to show", "<input name=nresults value='20'>");

    row2("", "<input type=submit value=\"OK\">\n");
    end_table();
    echo "</form>\n";

    page_tail();
?>
