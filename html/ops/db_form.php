<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/util_ops.inc");
require_once("../inc/db_ops.inc");

db_init();

$show_aggregate = false;
parse_str(getenv("QUERY_STRING"));

$title = table_title($table);
admin_page_head($title);

echo "<h2>Query $table table</h2>\n";
echo "<form method=\"get\" action=\"db_action.php\">\n";
echo "<p>\n";
echo "<input type=\"hidden\" name=\"table\" value=\"$table\">\n";

start_table();

switch($table){
	case "platform": 
		break;
	case "app": 
		break;
    case "app_version":
        print_detail_field();
        print_query_field();
        break;
    case "host":
        echo "
            <tr>
            <td align=\"right\">Show Aggregate Information</td>
            <td>
        ";
        print_checkbox("", "show_aggregate", $show_aggregate);
        echo "
            </td>
            </tr>
        ";
        print_detail_field();
        print_query_field();
        break;
    case "workunit":
        print_detail_field();
        print_query_field();
        echo "<input type=\"hidden\" name=\"sort_by\" value=\"id\">\n";
        break;
    case "result":
        echo "<tr><td align=\"right\">Server state</td><td> ";
        server_state_select();
        echo "</td></tr>\n";
        //print_text_field( "Batch number:", "batch", $batch );
        echo "<tr><td align=\"right\">Outcome</td><td>";
        outcome_select();
        echo "</td></tr>\n";
        echo "<tr><td align=\"right\">Client state</td><td>";
        client_state_select();
        echo "</td></tr>\n";
        echo "<tr><td align=\"right\">Validate state</td><td>";
        validate_state_select();
        echo "</td></tr>\n";
        echo "<tr><td align=\"right\">Sort by</td><td>";
        result_sort_select();
        sort_order_select();
        echo "</td></tr>\n";
        print_detail_field();
        print_query_field();
        break;
    case "team":
        print_query_field();
        break;
    case "user":
        print_query_field();
        break;
	default:
		echo "Unknown table name\n";
        exit();
}

row2("Number of entries to show", "<input name=\"nresults\" value=\"20\">");
row2("", "<input type=\"submit\" value=\"OK\">\n");
end_table();
echo "</form>\n";

print_describe_table($table, 4);

admin_page_tail();
?>
