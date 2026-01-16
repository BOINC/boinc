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

admin_page_head("Result summary per app version");

//   modified by Bernd Machenschalk 2007
//
//   1. distinguish between Darwin x86 and Darwin PPC
//   2. lists the "fail rates" for individual client states to allow for
//      distinguishing between download errors, computing errors and aborts
//   3. optionally list individual "unknown" OS by name
//   4. optionally list anonymous-platform versions
//
//   3. and 4. are probably rather confusing on open-source projects like SETI,
//   but I found them helpful e.g. on Einstein

$query_appid = get_int('appid');
$query_nsecs = get_int('nsecs');
$query_received_time = time() - $query_nsecs;
$query_all_versions = get_str('allversions', true);
$query_all_platforms = get_str('allplatforms', true);

$allplatforms = "";
$allversions = "";

if ($query_all_platforms == "1") {
    $unknown_platform = "host.os_name";
    $allplatforms = "checked";
} else {
    $unknown_platform = "'unknown'";
}
if ($query_all_versions == "1") {
    $limit_app_versions = "";
    $query_order = "platform";
    $allversions = "checked";
} else {
    // get the most recent version per (platform, plan class)
    //
    $app_versions = latest_avs_app($query_appid);
    $valid_app_versions = "";

    if (count($app_versions) > 0) {
        foreach ($app_versions as $av) {
            if (strlen($valid_app_versions) == 0) {
                $valid_app_versions = "$av->id";
            } else {
                $valid_app_versions .= ", $av->id";
            }
        }
        $limit_app_versions = "app_version_id IN ( $valid_app_versions ) AND";
        $query_order = "app_version_id DESC";
    } else {
        $limit_app_versions = "";
        $query_order = "platform";
        $allversions = "checked";
    }
}

// Now that we have a valid list of app_version_nums'
// let's construct the main query

$main_query = "
SELECT
       app_version_id,
       CASE
           when INSTR(host.os_name, 'Darwin')  then
                (CASE WHEN INSTR(host.p_vendor, 'Power') THEN 'Darwin PPC' ELSE 'Darwin x86' END)
           when INSTR(host.os_name, 'Linux')   then 'Linux'
           when INSTR(host.os_name, 'Windows') then 'Windows'
           when INSTR(host.os_name, 'SunOS')   then 'SunOS'
           when INSTR(host.os_name, 'Solaris') then 'Solaris'
           when INSTR(host.os_name, 'Mac')     then 'Mac'
           else $unknown_platform
       end AS platform,
       COUNT(*) AS total_results,
       ((SUM(case when outcome = 1 then 1 else 0 end) / COUNT(*)) * 100) AS pass_rate,
       ((SUM(case when outcome = 3 then 1 else 0 end) / COUNT(*)) * 100) AS fail_rate,
       ((SUM(case when outcome = 3 and client_state = 1 then 1 else 0 end) / COUNT(*)) * 100) AS fail_rate1,
       ((SUM(case when outcome = 3 and client_state = 2 then 1 else 0 end) / COUNT(*)) * 100) AS fail_rate2,
       ((SUM(case when outcome = 3 and client_state = 3 then 1 else 0 end) / COUNT(*)) * 100) AS fail_rate3,
       ((SUM(case when outcome = 3 and client_state = 4 then 1 else 0 end) / COUNT(*)) * 100) AS fail_rate4,
       ((SUM(case when outcome = 3 and client_state = 5 then 1 else 0 end) / COUNT(*)) * 100) AS fail_rate5,
       ((SUM(case when outcome = 3 and client_state = 6 then 1 else 0 end) / COUNT(*)) * 100) AS fail_rate6
FROM   result
           left join host on result.hostid = host.id
WHERE
       appid = $query_appid AND
       server_state = 5 AND
       $limit_app_versions
       received_time > $query_received_time
GROUP BY
       app_version_id,
       platform
ORDER BY
       $query_order
";

$db = BoincDb::get();
$result = $db->do_query($main_query);

start_table();
table_header(
    "App version", "Total<br>Results", "Pass Rate", "Fail Rate",
    "Failed<br>Downloading", "Failed<br>Downloaded", "Failed<br>Computing",
    "Failed<br>Uploading", "Failed<br>Uploaded", "Aborted"
);

while ($res = $result->fetch_object()) {
    $av = BoincAppVersion::lookup_id($res->app_version_id);
    if ($av) {
        $p = BoincPlatform::lookup_id($av->platformid);
        $ver_name = sprintf("%.2f", $av->version_num/100)." $p->name [$av->plan_class]";
    } else {
        $ver_name = "Anonymous ".$res->platform." ";
        switch ($res->app_version_id) {
        case -2: $ver_name .= " CPU"; break;
        case -3: $ver_name .= " NVIDIA GPU"; break;
        case -4: $ver_name .= " AMD GPU"; break;
        case -5: $ver_name .= " Intel GPU"; break;
        default: $ver_name .= " (unknown processor type)";
        }
    }
    echo "<tr>";

    echo "<td align=\"left\" valign=\"top\">$ver_name</td>";

    echo "<td align=\"right\" valign=\"top\">";
    echo $res->total_results;
    echo "&nbsp;&nbsp;</td>";

    echo "<td align=\"right\" valign=\"top\">";
    echo $res->pass_rate;
    echo "%&nbsp;&nbsp;</td>";

    echo "<td align=\"right\" valign=\"top\">";
    echo $res->fail_rate;
    echo "%&nbsp;&nbsp;</td>";

    echo "<td align=\"right\" valign=\"top\">";
    echo $res->fail_rate1;
    echo "%&nbsp;&nbsp;</td>";

    echo "<td align=\"right\" valign=\"top\">";
    echo $res->fail_rate2;
    echo "%&nbsp;&nbsp;</td>";

    echo "<td align=\"right\" valign=\"top\">";
    echo $res->fail_rate3;
    echo "%&nbsp;&nbsp;</td>";

    echo "<td align=\"right\" valign=\"top\">";
    echo $res->fail_rate4;
    echo "%&nbsp;&nbsp;</td>";

    echo "<td align=\"right\" valign=\"top\">";
    echo $res->fail_rate5;
    echo "%&nbsp;&nbsp;</td>";

    echo "<td align=\"right\" valign=\"top\">";
    echo $res->fail_rate6;
    echo "%&nbsp;&nbsp;</td>";

    echo "</tr>\n";

}
$result->free();

echo "</table>\n";

$page = $_SERVER["REQUEST_URI"];
echo "<form action=$page>\n";
echo "<input type=\"hidden\" name=\"appid\" value=\"$query_appid\">\n";
echo "<input type=\"hidden\" name=\"nsecs\" value=\"$query_nsecs\">\n";
echo "<input type=\"checkbox\" name=\"allversions\" value=\"1\" $allversions>\n";
echo "show anonymous platform versions&nbsp;&nbsp;\n";
echo "<input type=\"checkbox\" name=\"allplatforms\" value=\"1\" $allplatforms>\n";
echo "distinguish unknown platforms&nbsp;&nbsp;\n";
echo "<input class=\"btn btn-default\" type=\"submit\" value=\"show\">\n";
echo "</form>\n";

admin_page_tail();

?>
