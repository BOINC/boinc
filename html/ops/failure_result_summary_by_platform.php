<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/util_ops.inc");

db_init();
admin_page_head("Result Failure Summary by Platform");

$query_appid = $_GET['appid'];
$query_received_time = time() - $_GET['nsecs'];

$q = new SqlQueryString();
$q->process_form_items();

$main_query = "
SELECT
    app_version_num AS App_Version,
    case
        when INSTR(host.os_name, 'Darwin') then 'Darwin'
        when INSTR(host.os_name, 'Linux') then 'Linux'
        when INSTR(host.os_name, 'Windows') then 'Windows'
        when INSTR(host.os_name, 'SunOS') then 'SunOS'
        when INSTR(host.os_name, 'Solaris') then 'Solaris'
        when INSTR(host.os_name, 'Mac') then 'Mac'
        else 'Unknown'
    end AS OS_Name,
    exit_status,
    COUNT(*) AS error_count
FROM   result
        left join host on result.hostid = host.id
WHERE
    appid = '$query_appid' and
    server_state = '5' and
    outcome = '3' and
    received_time > '$query_received_time'
GROUP BY
    app_version_num DESC,
    OS_Name,
    exit_status
";

$urlquery = $q->urlquery;
$result = mysql_query($main_query);

echo "<table>\n";
echo "<tr><th>App Version</th><th>OS</th><th>Exit Status</th><th>Error Count</th></tr>\n";

while ($res = mysql_fetch_object($result)) {

    echo "<tr>";

    echo "<td align=\"left\" valign=\"top\">";
    echo $res->App_Version;
    echo "</td>";

    echo "<td align=\"left\" valign=\"top\">";
    echo $res->OS_Name;
    echo "</td>";

    echo "<td align=\"left\" valign=\"top\">";
    $exit_status_condition = "exit_status=$res->exit_status";
    echo link_results(exit_status_string($res), $urlquery, "$exit_status_condition", "");
    echo "</td>";

    echo "<td align=\"left\" valign=\"top\">";
    echo $res->error_count;
    echo "</td>";

    echo "</tr>\n";

}
mysql_free_result($result);

echo "</table>\n";

admin_page_tail();

?>
