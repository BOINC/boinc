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
admin_page_head("Failures grouped by app version and host");

$query_appid = $_GET['appid'];
$query_received_time = time() - $_GET['nsecs'];

$main_query = "
SELECT
       app_version_id,
       app_version_num,
       hostid AS Host_ID,
       case
           when INSTR(host.os_name, 'Darwin') then 'Darwin'
           when INSTR(host.os_name, 'Linux') then 'Linux'
           when INSTR(host.os_name, 'Windows') then 'Windows'
           when INSTR(host.os_name, 'SunOS') then 'SunOS'
           when INSTR(host.os_name, 'Solaris') then 'Solaris'
           when INSTR(host.os_name, 'Mac') then 'Mac'
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
       app_version_id,
       hostid
order by error_count desc
";

$result = mysql_query($main_query);

start_table();
table_header(
    "App version", "Host ID", "OS Version", "Results today",
    "Error count"
);

while ($res = mysql_fetch_object($result)) {
    $av = BoincAppVersion::lookup_id($res->app_version_id);
    $p = BoincPlatform::lookup_id($av->platformid);
    table_row(
        sprintf("%.2f", $res->app_version_num/100)." $p->name [$av->plan_class]",
        $res->Host_ID,
        $res->OS_Version, $res->Results_Today,
        "<a href=db_action.php?table=result&detail=low&hostid=$res->Host_ID&app_version_id=$res->app_version_id&server_state=5&outcome=3>$res->error_count</a>"
    );
}
mysql_free_result($result);

end_table();

admin_page_tail();

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
