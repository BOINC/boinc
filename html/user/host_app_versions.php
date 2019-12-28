<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

function rsc_name($t) {
    switch ($t) {
    case 2: return tra("CPU");
    case 3: return tra("NVIDIA GPU");
    case 4: return tra("AMD GPU");
    case 5: return tra("Intel GPU");
    }
    return tra("Unknown");
}

function av_desc($gavid, $show_dep) {
    if ($gavid >= 1000000) {
        // anonymous platform
        //
        $appid = (int)($gavid/1000000);
        $app = BoincApp::lookup_id($appid);
        if (!$app) {
            return null;
        }
        if (!$show_dep && $app->deprecated) {
            return null;
        }
        $rsc_type = $gavid % 1000000;
        $r = rsc_name($rsc_type);
        return "$app->user_friendly_name (".tra("anonymous platform").", $r)";
    } else {
        $av = BoincAppVersion::lookup_id($gavid);
        if (!$av) {
            return null;
        }
        if (!$show_dep && $av->deprecated) {
            return null;
        }
        $app = BoincApp::lookup_id($av->appid);
        if (!$app) {
            return null;
        }
        if (!$show_dep && $app->deprecated) {
            return null;
        }
        $platform = BoincPlatform::lookup_id($av->platformid);
        if (!$platform) return tra("Missing platform");
        $pc = (strlen($av->plan_class))?"($av->plan_class)":"";
        $v = number_format($av->version_num/100, 2);
        return "$app->user_friendly_name $v $platform->name $pc";
    }
}

function show_hav($hav, $show_dep) {
    $desc = av_desc($hav->app_version_id, $show_dep);
    if (!$desc) return;
    row1($desc);
    row2(tra("Number of tasks completed"), $hav->et_n);
    row2(tra("Max tasks per day"), $hav->max_jobs_per_day);
    row2(tra("Number of tasks today"), $hav->n_jobs_today);
    row2(tra("Consecutive valid tasks"), $hav->consecutive_valid);
    $x = number_format($hav->turnaround_avg/86400, 2);
    if ($hav->et_avg) {
        $gflops = number_format(1e-9/$hav->et_avg, 2);
        row2(tra("Average processing rate"), $gflops." GFLOPS");
    }
    row2(tra("Average turnaround time"), "$x days");
}

$hostid = get_int('hostid');
$show_dep = get_int('show_dep', true);

$havs = BoincHostAppVersion::enum("host_id=$hostid");

page_head(tra("Application details for host %1", $hostid));
start_table();
foreach ($havs as $hav) {
    //if (!$hav->pfc_n) continue;
    show_hav($hav, $show_dep);
}
end_table();
if ($show_dep) {
    show_button("host_app_versions.php?hostid=$hostid", "Show active versions");
} else {
    show_button("host_app_versions.php?hostid=$hostid&show_dep=1", "Show all versions");
}

page_tail();
?>
