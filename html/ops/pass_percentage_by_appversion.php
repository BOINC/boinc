<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2016 University of California
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

$query_appid = get_int('appid');
$query_nsecs = get_int('nsecs', true);
$query_received_time = time() - $query_nsecs;
$query_all_versions = get_str('allversions', true);

$allversions = "";

if ($query_nsecs) {
    $received_time = time() - $query_nsecs;
    $limit_received_time = " AND received_time > '$received_time'";
} else {
    $limit_received_time = "";
}

$query_order = "platform, version DESC, plan_class";

if ($query_all_versions == "1") {
    $limit_app_versions = "";
    $allversions = "checked";
} else {
    $allversions = "";
    // get the most recent version per (platform, plan class)
    //
    $app_versions = latest_avs_app($query_appid);
    $valid_app_versions = "";

    if (count($app_versions) > 0) {
        foreach ($app_versions as $av) {
            if (strlen($valid_app_versions) == 0) {
                $valid_app_versions = "$av->version_num";
            } else {
                $valid_app_versions .= ", $av->version_num";
            }
        }
        $limit_app_versions = " AND app_version_num IN ( $valid_app_versions )";
        //$query_order = "version DESC";
    } else {
        $limit_app_versions = "";
        //$query_order = "platform";
        $allversions = "checked";
    }
}

// Now that we have a valid list of app_version_nums'
// let's construct the main query

$main_query = "
SELECT
       app_version_id,
       app_version_num AS version,
       platform_name.name AS platform,
       plan_class,
       COUNT(*) AS total_results,
       (SUM(case when outcome = '1' then 1 else 0 end)) AS pass_count,
       (SUM(case when outcome = '3' then 1 else 0 end)) AS fail_count,
       (SUM(case when outcome = '3' and client_state = '1' then 1 else 0 end)) AS fail_count1,
       (SUM(case when outcome = '3' and client_state = '2' then 1 else 0 end)) AS fail_count2,
       (SUM(case when outcome = '3' and client_state = '3' then 1 else 0 end)) AS fail_count3,
       (SUM(case when outcome = '3' and client_state = '4' then 1 else 0 end)) AS fail_count4,
       (SUM(case when outcome = '3' and client_state = '5' then 1 else 0 end)) AS fail_count5,
       (SUM(case when outcome = '3' and client_state = '6' then 1 else 0 end)) AS fail_count6,
       (SUM(case when outcome = '6' then 1 else 0 end)) AS fail_count7
FROM   result LEFT JOIN (SELECT app_version.id AS id,
                                plan_class,
                                platform.name
                         FROM app_version INNER JOIN platform
                                          ON platform.id = platformid
                        ) as platform_name
              ON app_version_id = platform_name.id
WHERE
       appid = '$query_appid' AND
       server_state = '5' AND outcome NOT IN (4,5)
       $limit_app_versions
       $limit_received_time
GROUP BY
       app_version_num,
       app_version_id DESC
ORDER BY
       $query_order
";

echo "<!-- QUERY: $main_query -->";

$db = BoincDb::get(true);
$result = $db->do_query($main_query);

if($db->base_errno()) {
    echo $db->base_error();
}

start_table();
table_header(
    "App version", "Platform", "Plan Class", "Total<br>Results", "Pass Rate", "Fail Rate",
    "Failed<br>Downloading", "Failed<br>Downloaded", "Failed<br>Computing",
    "Failed<br>Uploading", "Failed<br>Uploaded", "Aborted", "Validate<br>errors"
);

while ($res = $result->fetch_object()) {
    echo "<tr>";

    echo "  <td align=\"left\" valign=\"top\">";
    echo $res->version;
    echo " / ";
    echo $res->app_version_id;
    echo "</td>\n";

    echo "  <td align=\"left\" valign=\"top\">";
    if($res->platform)
        echo $res->platform;
    else
        echo "unknown";
    echo "</td>\n";

    echo "  <td align=\"left\" valign=\"top\">";
    echo $res->plan_class;
    echo "</td>\n";

    $tot = $res->total_results;

    echo "  <td align=\"right\" valign=\"top\">";
    echo $tot;
    echo "&nbsp;&nbsp;</td>\n";

    $abs = $res->pass_count;
    $perc = $abs / $tot * 100;
    echo "  <td align=\"right\" valign=\"top\">";
    printf("<div title=\"%d\">%.4f%%</div>&nbsp;&nbsp;", $abs, $perc);
    echo "</td>\n";

    $abs = $res->fail_count;
    $perc = $abs / $tot * 100;
    echo "  <td align=\"right\" valign=\"top\">";
    printf("<div title=\"%d\">%.4f%%</div>&nbsp;&nbsp;", $abs, $perc);
    echo "</td>\n";

    $abs = $res->fail_count1;
    $perc = $abs / $tot * 100;
    echo "  <td align=\"right\" valign=\"top\">";
    printf("<div title=\"%d\">%.4f%%</div>&nbsp;&nbsp;", $abs, $perc);
    echo "</td>\n";

    $abs = $res->fail_count2;
    $perc = $abs / $tot * 100;
    echo "  <td align=\"right\" valign=\"top\">";
    printf("<div title=\"%d\">%.4f%%</div>&nbsp;&nbsp;", $abs, $perc);
    echo "</td>\n";

    $abs = $res->fail_count3;
    $perc = $abs / $tot * 100;
    echo "  <td align=\"right\" valign=\"top\">";
    if ($abs > 0) {
        echo "<a title=\"$abs\" href=\"ordered_client_errors.php?appid=$query_appid&nsecs=$query_nsecs&appverid=$res->app_version_id\">";
        printf("%.4f%%</a>", $perc);
    } else {
        printf("<div title=\"%d\">%.4f%%</div>&nbsp;&nbsp;", $abs, $perc);
    }
    echo "</td>\n";

    $abs = $res->fail_count4;
    $perc = $abs / $tot * 100;
    echo "  <td align=\"right\" valign=\"top\">";
    printf("<div title=\"%d\">%.4f%%</div>&nbsp;&nbsp;", $abs, $perc);
    echo "</td>\n";

    $abs = $res->fail_count5;
    $perc = $abs / $tot * 100;
    echo "  <td align=\"right\" valign=\"top\">";
    printf("<div title=\"%d\">%.4f%%</div>&nbsp;&nbsp;", $abs, $perc);
    echo "</td>\n";

    $abs = $res->fail_count6;
    $perc = $abs / $tot * 100;
    echo "  <td align=\"right\" valign=\"top\">";
    printf("<div title=\"%d\">%.4f%%</div>&nbsp;&nbsp;", $abs, $perc);
    echo "</td>\n";

    $abs = $res->fail_count7;
    $perc = $abs / $tot * 100;
    echo "  <td align=\"right\" valign=\"top\">";
    if ($abs > 0) {
        echo "<a title=\"$abs\" href=\"db_action.php?table=result&server_state=5&outcome=6&detail=low&nresults=20&clauses=app_version_id+%3D+$res->app_version_id\">";
        printf("%.4f%%</a>", $perc);
    } else {
        printf("<div title=\"%d\">%.4f%%</div>&nbsp;&nbsp;", $abs, $perc);
    }
    echo "</td>\n";

    echo "</tr>\n";

}

$result->free();

echo "</table>\n";

$page = $_SERVER["REQUEST_URI"];
echo "<form action=$page>\n";
echo "<input type=\"hidden\" name=\"appid\" value=\"$query_appid\">\n";
echo "<input type=\"hidden\" name=\"nsecs\" value=\"$query_nsecs\">\n";
echo "<input type=\"checkbox\" name=\"allversions\" value=\"1\" $allversions>\n";
echo "list all reported App versions&nbsp;&nbsp;\n";
echo "<input class=\"btn btn-default\" type=\"submit\" value=\"show\">\n";
echo "</form>\n";

admin_page_tail();

echo "\n";

?>
