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

BoincDb::get(true);

function main() {
    if (get_str('xml', true)) {
        show_apps_xml();
        return;
    }
    $avid = get_int('avid', true);
    if ($avid) {
        show_app_version($avid);
        return;
    }
    $app_id = get_int('app_id', true);
    if ($app_id) {
        show_app($app_id);
        return;
    }
    show_apps();
}

function show_apps_xml() {
    $platforms = BoincPlatform::enum("deprecated=0");
    require_once('../inc/xml.inc');
    xml_header();
    echo "<app_versions>\n";
    $apps = BoincApp::enum("deprecated=0");
    foreach ($apps as $app) {
        echo "<application>\n";
        echo "    <user_friendly_name>$app->user_friendly_name</user_friendly_name>\n";
        echo "    <name>$app->name</name>\n";
        echo "    <id>$app->id</id>\n";
        if ($app->beta) {
            echo "        <beta/>\n";
        }
        foreach ($platforms as $platform) {
            $avs = latest_avs_app_platform($app->id, $platform->id);
            foreach($avs as $av) {
                $create_time_f = pretty_time_str($av->create_time);
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
            }
        }
        echo "    </application>\n";
    }
    echo "</app_versions>\n";
}

function show_apps() {
    page_head(tra("Applications"));
    echo tra("%1 currently has the following applications. When you participate in %1, tasks for one or more of these applications will be assigned to your computer. The current version of the application will be downloaded to your computer. This happens automatically; you don't have to do anything.", PROJECT)."<br><br>
    ";

    $apps = BoincApp::enum("deprecated=0");
    start_table('table-striped');
    row_heading_array([
        'Name<br><small>Click for details</small>',
        'Created',
        'Recent average GFLOPS',
    ]);
    foreach ($apps as $app) {
        $b = $app->beta?' (beta test)':'';
        $gf = BoincAppVersion::sum('expavg_credit', "where appid=$app->id")/200;
        row_array([
            "<a href=apps.php?app_id=$app->id>$app->user_friendly_name$b</a>",
            pretty_time_str($app->create_time),
            number_format($gf, 2)
        ]);
    }
    end_table();
    page_tail();
}

function show_app($app_id) {
    $app = BoincApp::lookup_id($app_id);
    if (!$app) error_page('no app');
    page_head("$app->user_friendly_name");
    echo "<h2>Versions</h2>";
    start_table('table-striped');
    $platforms = BoincPlatform::enum("deprecated=0");
    $total_gf = 0;
    row_heading_array([
        'ID',
        'Platform',
        'Version',
        'Plan class',
        'Created',
        'Recent average GFLOPS'
    ]);
    foreach ($platforms as $platform) {
        $avs = latest_avs_app_platform($app->id, $platform->id);
        foreach($avs as $av) {
            $create_time_f = pretty_time_str($av->create_time);
            $version_num_f = sprintf("%0.2f", $av->version_num/100);
            $gf = $av->expavg_credit/200;
            $total_gf += $gf;
            $gf = number_format($gf, 2);
            $b = $av->beta?" (beta test)":"";
            row_array([
                "<a href=apps.php?avid=$av->id>$av->id</a>",
                $platform->user_friendly_name,
                "$version_num_f$b",
                $av->plan_class,
                $create_time_f,
                $gf
            ]);
        }
    }
    end_table();
    $x = number_format($total_gf, 2);
    echo "<p>Total average computing: $x GigaFLOPS";
    page_tail();
}

function show_app_version($avid) {
    $av = BoincAppVersion::lookup_id($avid);
    if (!$av) error_page('no app version');
    $app = BoincApp::lookup_id($av->appid);
    $platform = BoincPlatform::lookup_id($av->platformid);
    page_head("App version $avid");
    start_table('table-striped');
    row2('App', $app->user_friendly_name);
    row2('Version', $av->version_num);
    row2('Platform', $platform->user_friendly_name);
    row2('Plan class', $av->plan_class);
    row2('Created', pretty_time_str($av->create_time));
    row2("pfc_n", $av->pfc_n);
    row2("pfc_avg", number_format($av->pfc_avg, 3));
    row2("pfc_scale", number_format($av->pfc_scale, 3));
    row2("expavg_credit", number_format($av->expavg_credit, 3));
    row2('expavg_time', pretty_time_str($av->expavg_time));
    end_table();
    page_tail();
}

main();

?>
