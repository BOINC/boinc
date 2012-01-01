<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

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
    app_version.plan_class AS Plan_Class,
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
        left join app_version on result.app_version_id = app_version.id
WHERE
    result.appid = '$query_appid' and
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
echo "<tr><th>App Version</th><th>Plan Class</th><th>OS</th><th>Exit Status</th><th>Error Count</th></tr>\n";

while ($res = mysql_fetch_object($result)) {

    echo "<tr>";

	echo "<td align=\"left\" valign=\"top\">";
	echo $res->App_Version;
	echo "</td>";

	echo "<td align=\"left\" valign=\"top\">";
	echo $res->Plan_Class;
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

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
