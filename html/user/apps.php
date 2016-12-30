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

require_once("../inc/util.inc");

check_get_args(array("xml"));

BoincDb::get(true);
$platforms = BoincPlatform::enum("deprecated=0");

$xml = get_str('xml', true);
if ($xml) {
    require_once('../inc/xml.inc');
    xml_header();
    echo "<app_versions>\n";
} else {
    page_head(tra("Applications"));
    echo tra("%1 currently has the following applications. When you participate in %1, tasks for one or more of these applications will be assigned to your computer. The current version of the application will be downloaded to your computer. This happens automatically; you don't have to do anything.", PROJECT)."<br><br>
    ";
    start_table('table-striped');
}

$apps = BoincApp::enum("deprecated=0");

$total_gf = 0;
foreach ($apps as $app) {
    if ($xml) {
        echo "<application>\n";
        echo "    <user_friendly_name>$app->user_friendly_name</user_friendly_name>\n";
        echo "    <name>$app->name</name>\n";
        echo "    <id>$app->id</id>\n";
        if ($app->beta) {
            echo "        <beta/>\n";
        }
    } else {
        $b = $app->beta?" (beta test)":"";
        echo "
            <tr><th class=\"bg-primary\" colspan=4>$app->user_friendly_name$b</th></tr>
            <tr>
                <th>".tra("Platform")."</th>
                <th>".tra("Version")."</th>
                <th>".tra("Created")."</th>
                <th>".tra("Average computing")."</th>
            </tr>
        ";
    }
    foreach ($platforms as $platform) {
        $avs = latest_avs_app_platform($app->id, $platform->id);
        foreach($avs as $av) {
            $create_time_f = pretty_time_str($av->create_time);
            if ($xml) {
                echo "    <version>\n";
                echo "        <platform_short>$platform->name</platform_short>\n";
                echo "        <platform_long>$platform->user_friendly_name</platform_long>\n";
                echo "        <version_num>$av->version_num</version_num>\n";
                echo "        <plan_class>$av->plan_class</plan_class>\n";
                echo "        <date>$create_time_f</date>\n";
                echo "        <date_unix>$av->create_time</date_unix>\n";
                if ($av->beta) {
                    echo "        <beta/>\n";
                }
                echo "    </version>\n";
            } else {
                $version_num_f = sprintf("%0.2f", $av->version_num/100);
                if ($av->plan_class) {
                    $version_num_f .= " ($av->plan_class)";
                }
                $gf = $av->expavg_credit/200;
                $total_gf += $gf;
                $gf = number_format($gf, 0);
                $b = $av->beta?" (beta test)":"";
                echo "<tr>
                    <td>$platform->user_friendly_name</td>
                    <td>$version_num_f$b</td>
                    <td>$create_time_f</td>
                    <td align=right>$gf GigaFLOPS</td>
                    </tr>
                ";
            }
        }
    }
    if ($xml) {
        echo "    </application>\n";
    }
}

if ($xml) {
    echo "</app_versions>\n";
} else {
    end_table();
    $x = number_format($total_gf, 0);
    echo "<p>Total average computing: $x GigaFLOPS";
    page_tail();
}
?>
