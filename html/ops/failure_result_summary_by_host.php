<?php

require_once("../inc/util_ops.inc");

db_init();
page_head("Result Failure Summary by Host");


$query_appid = $_GET['appid'];
$query_received_time = time() - $_GET['nsecs'];

$main_query = "
SELECT
       app_version_num AS App_Version,
       hostid AS Host_ID,
       case
           when INSTR(host.os_name, 'Darwin') then 'Darwin'
           when INSTR(host.os_name, 'Linux') then 'Linux'
           when INSTR(host.os_name, 'Windows') then 'Windows'
           when INSTR(host.os_name, 'SunOS') then 'SunOS'
           else 'Unknown'
       end AS OS_Name,
       case
           when INSTR(host.os_name, 'Linux') then 
               case
                   when INSTR(LEFT(host.os_version, 6), '-') then LEFT(host.os_version, (INSTR(LEFT(host.os_version, 6), '-') - 1))
                   else LEFT(host.os_version, 6)
               end
           else host.os_version
       end AS OS_Version,
       host.nresults_today AS Results_Today, 
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
       hostid,
       OS_Name,
       OS_Version,
       host.nresults_today
";

$result = mysql_query($main_query);

echo "<table>\n";
echo "<tr><th>App Version</th><th>Host ID</th><th>OS Name</th><th>OS Version</th><th>Results Today</th><th>Error Count</th></tr>\n";

while ($res = mysql_fetch_object($result)) {

    echo "<tr>";
    
    echo "<td align=left valign=top>";
    echo $res->App_Version;
    echo "</td>";

    echo "<td align=left valign=top>";
    echo $res->Host_ID;
    echo "</td>";

    echo "<td align=left valign=top>";
    echo $res->OS_Name;
    echo "</td>";

    echo "<td align=left valign=top>";
    echo $res->OS_Version;
    echo "</td>";

    echo "<td align=left valign=top>";
    echo $res->Results_Today;
    echo "</td>";

    echo "<td align=left valign=top>";
    echo $res->error_count;
    echo "</td>";

    echo "</tr>\n";

}
mysql_free_result($result);

echo "</table>\n";

page_tail();

?>
